// Светильник многофункиональный Н4, версия 2.0

//--------------Настройки-----------------
#define LED_NUM 12     // количество светодиодов
#define COLOR_COUNT 10 // количество цветов
#define IR_CMD 24      // количество команд пульта
#define FADE_IN 30     // коэффициент плавности зажигания (чем ниже, тем плавнее)
#define FADE_OUT 10    // коэффициент плавности затухания (чем ниже, тем плавнее)

#define HIGH_VCC 4200  // верхний порог заряда аккумулятора
#define LOW_VCC 3000   // нижний порог заряда аккумулятора
#define FPS 50         // частота обновления
#define DEBUG 0        // вывод данный в Serial
#define SAVE_TIME 60   // время сохранения после приёма последней команды с пульта

//---------------Пины---------------------
#define CLK 7          // пин CLK дисплея
#define DIO 8          // пин DIO дисплея
#define IR_PIN 3       // пин ИК-приёмника
#define IR_VCC 2       // пин питания ИК-приёмника
#define LED_PIN A3     // пин светодиодов
#define BEEP_PIN 10    // пин пищалки
#define BUTTON_PREV A2 // пин кнопки "Назад"
#define BUTTON_NEXT 9  // пин кнопки "Вперёд"
#define RTC_VCC 6      // пин питания часов
#define MIC_PIN A6     // пин аудиовхода
#define CHG_PIN A1     // пин индикатора зарядки

//--------------Дэфайны-------------------
#define LOG_OUT 1
#define FHT_N 64 // удвоенное количество каналов (т.е., на самом деле, их 32)

//Дичь для увеличения частоты АЦП
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define MODES 8 // количество режимов

#define CHG_COL 144 // цвет зарядки

#define DK (1000/FPS) // время одного кадра

//Названия режимов
#define OFF_MODE 0
#define COLOR_MODE 1
#define GRADIENT_MODE 2
#define RAINBOW_MODE 3
#define MUSIC_MODE 4
#define CLOCK_MODE 5
#define OPT_MODE 6
#define CHG_MODE 7

//Структура памяти
#define EXIST 0
#define EXIST_NUM 0x0f
#define SOUND 1
#define LED 2
#define DISP 3
#define TR_TYPE 4
#define TR_TIME 5
#define ALARM 6
#define TIME 7
#define HUE_C (TIME+3)
#define SAT_C (HUE_C+COLOR_COUNT)
#define IR_DAT (SAT_C+COLOR_COUNT)
#define HUE_M (IR_DAT+IR_CMD*4)
#define SAT_M (HUE_M+LED_NUM)

#define HB_DAY 20
#define HB_MON 9




//-------------Библиотеки-----------------
#include <EEPROM.h>             // флеш-память
#include "PinChangeInterrupt.h" // прерывания для ИК
#include "IRLremote.h"          // ИК-приёмник
#include <FastLED.h>            // светодиоды
#include <DS3231.h>             // часы
#include <FHT.h>                // быстрое преобразование Хартли для анализа звука
#include "TM1637.h"             // дисплей
#include "buttonMinim.h" 
//#include "LowPower.h"

double prevVolts = 100.0;



//-----------Объекты----------------------
TM1637 disp(CLK, DIO);
CHashIR IRLremote;
DS3231  Clock(SDA, SCL);
Time clk;
CRGB leds[LED_NUM];
buttonMinim prevBut(BUTTON_PREV);
buttonMinim nextBut(BUTTON_NEXT);

//--------Переменные---------------------
byte mode = 0,                 // режим
     submode = 0,              // подрежим
     last_mode = 0,            // последний режим
     low = 0,                  // нижний порог звука, автоматически настраивается при включении
     bright_led = 255,         // яркость светодиодов
     bright_display = 1,       // яркость дисплея
     fade = 3,                 // время перехода, сотни мс
     transfusion = 10,         // время одного круга эффекта, сотни мс
     transit_type = 1,         // тип перехода
     display_time = 5,         // время горения дисплея
     fade_in_time = 30,        // коэффициент плавности зажигания (чем ниже, тем плавнее)
     fade_out_time = 9,        // коэффициент плавности затухания (чем ниже, тем плавнее)
     opt_val = 0;              // переменные при настройке

     
unsigned long display_timer,    // таймер дисплея
              ir_timer,
              chg_timer = 0;

byte colors[2][10];            // цвета по умолчанию
byte colors_music[2][LED_NUM]; // цвета цветомузыки
unsigned long ir_data[24];     // данные пульта

byte alarm_time[] = {0, 0, 0}; // будильник (часы, минуты, время "рассвета")
bool alarm = false,            // включение будильника
     sound = true,             // звук
     displayed = false,        // включён ли дисплей
     ir_rec = false,           // принят ли сигнал с пульта
     alarm_rst = false,
     sun = false,
     played = false;      

byte submodes[] = {0,COLOR_COUNT,2,2,1,3,2,1},
     opt_count[] = {0, 3};
uint8_t dsp_data[4] = {_S,_U,_b, 0};

uint8_t nameModes[MODES][4] = {
{0,_O,_F,_F},
{0,_C,_O,_L},
{_G,_r,_A,_d},
{0, _r,_A,_d},
{0,_N,_U,_S},
{0,_C,_L,_C},
{0,_O,_P,_t}};

void setup() {
  start();
}

void loop() {
  buttonRead();
  irRead();
  displayUpd();
  if (alarm) checkAlarm();
  clk = Clock.getTime();
  Celebration();

  if (ADCSRA << ADEN && mode != MUSIC_MODE && mode != CHG_MODE) ADCSRA &= ~(1 << ADEN); // Отключаем АЦП
  
  //Serial.println(alarm);
  switch (mode){
    case OFF_MODE:
    if (!sun) transit(CRGB::Black, transit_type); break;

    case COLOR_MODE:
    transit(CHSV(colors[0][submode], colors[1][submode], bright_led), transit_type); break;

    case GRADIENT_MODE:
    fadeEffect(submode==1);  break;

    case RAINBOW_MODE:
    rainbowEffect(submode==1);  break;
    
    case MUSIC_MODE:
    colormusic(); break;

    case CLOCK_MODE:
    clockMode(); break;

    case OPT_MODE:
    setOptions(); break;

    case CHG_MODE:
    showCharge(); break;
    
  }

}
