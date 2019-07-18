//Библиотеки


#include <Config.h>

#include "LowPower.h"  // энергосбережение

#include <EEPROM.h>    // флеш-память

// ИК-приёмник
#include "PinChangeInterrupt.h"
#include "IRLremote.h"
CNec IRLremote;

#include <FastLED.h> // светодиоды

// Часы
#include <DS3231.h>
DS3231  Clock(SDA, SCL);
Time  t;

// Дисплей
#define CLK 11
#define DIO 12
#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

// Вывод отладочных данных в Serial
#define DEBUG 0

// Быстрое преобразование Хартли для анализа звука
#define LOG_OUT 1
#define FHT_N 64 //Удвоенное количество каналов (т.е., на самом деле, их 64)
#include <FHT.h>
double prevVolts = 100.0;
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

byte bright = 10;       //яркость светодиодов
byte bright_screen = 7; //яркость дисплея

#define IR_PIN 3       // пин ИК-приёмника
#define IR_VCC 2       // пин питания ИК-приёмника
#define LED_PIN A3     // пин светодиодов
#define BEEP_PIN A1    // пин пищалки
#define BUTTON_PREV A2 // пин кнопки "Назад"
#define BUTTON_NEXT 10 // пин кнопки "Вперёд"
#define RTC_VCC 9      // пин питания часов
#define MIC_PIN A6     // пин аудиовхода



#define LED_NUM 12     // количество светодиодов
CRGB leds[LED_NUM];    // массив со светодиодами

byte modes = 16;       // количество режимов
byte mode = 0;         // режим по умолчанию


byte fade_in_time = 30; // коэффициент плавности зажигания (чем ниже, тем плавнее)
byte fade_out_time = 9; // коэффициент плавности затухания (чем ниже, тем плавнее)

byte low = 0;           // нижний порог звука, автоматически настраивается при включении

boolean mute = false;   // выключение пищалки

byte colors[2][8] = {{0, 32, 64, 96, 128, 160, 192, 0}, {255, 255, 255, 255, 255, 255, 255, 0}}; // цвета по умолчанию
byte colors_music[2][12] = {{0,   16,   32,  64,  80,  96, 112, 128, 144, 160, 176, 192},
  {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}};       // цвета цветомузыки

byte alarm_time[] = {0, 0, 0}; // будильник (часы, минуты, время "рассвета")
boolean alarm = false;         // включение будильника

unsigned int wait = 3000;      //скорость переливания цвета
byte mov = 0;

byte low_chg = 7;              // значение низкого заряда в %
byte type_clock = 0;           // тип часов (обычные, с эффектом вращения, с эффектом прокрутки)

boolean timer_color = false;

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif

  //Увеличение частоты АЦП
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  //Назначение пинов
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(RTC_VCC, OUTPUT);
  pinMode(IR_VCC, OUTPUT);
  pinMode(13, OUTPUT);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_NUM);
  IRLremote.begin(IR_PIN);

  // Включение часов и ИК
  digitalWrite(RTC_VCC, HIGH);
  digitalWrite(IR_VCC, HIGH);

  //Снижение напряжения АЦП
  analogReference(INTERNAL);

  //Запуск часов
  Clock.begin();

  if (EEPROM.read(100) == 100) {         //если были сохранены настройки, загрузить их
    for (byte i = 0; i < 8; i++) {
      colors[0][i] = EEPROM.read(i);
      colors[1][i] = EEPROM.read(i + 8);
    }

    for (byte i = 0; i < 12; i++) {
      colors_music[0][i] = EEPROM.read(i + 22);
      colors_music[1][i] = EEPROM.read(i + 34);
    }
    bright = EEPROM.read(16);
    mute = EEPROM.read(17);
    alarm_time[0] = EEPROM.read(18);
    alarm_time[1] = EEPROM.read(19);
    alarm_time[2] = EEPROM.read(21);
    alarm = EEPROM.read(20);
    bright_screen = EEPROM.read(50);
  }
  else {
    EEPROM.write(100, 100);
  }

  disp.clear();
  disp.brightness(bright_screen);  // яркость, 0 - 7 (минимум - максимум)

  //Расчёт напряжения и вывод на светодиоды
  int napr = constrain(map(readVcc(), 3000, 4200, 0, 96), 0, 96);
  setcolorall(napr, 255, 100);

  //Приветственный текст на дисплее
  disp.displayByte(_H, 0x30, _N, _4);
  disp.point(1);

  //Приветственный сигнал, если не выключен звук
  if (!mute) {
    three_beep(500, 750, 1000, 200);
    tone(BEEP_PIN, 1000, 300);
  }

  //Очистка дисплея
  delay(2000);
  disp.clear();
  disp.point(0);

  //Сброс шумов
  denoise();

}


byte max_val_f = 50;
float k_v = 0.1;
float k_c = 0.7;
byte flag[] = {0, 0};   //Флаги кнопок
byte last_mode = 0;     //Недавний режим, включается при нажатии кнопки "ON" на пульте
boolean sun = false;    //Режим "рассвета"

void loop() {




  //Изменине режима при нажатии кнопки "Вперёд"
  if (digitalRead(BUTTON_NEXT) == LOW && flag[0] == 0) {
    flag[0] = 1; //Изменеие значение флага (чтобы действие выполнялось один раз при нажатии + защита от дребезга)
  }
  if (digitalRead(BUTTON_NEXT) == HIGH && flag[0] == 1) {      //Действия после отпускания кнопки:
    mode = (mode + 1) % (modes + 1);                           // Переключение режима
    flag[0] = 0;
  }                                              //Сброс флага кнопки


  //Действия при нажатии кнопки "Назад"
  if (digitalRead(BUTTON_PREV) == LOW && flag[1] == 0) {
    flag[1] = 1; //Изменеие значение флага (чтобы действие выполнялось один раз при нажатии + защита от дребезга)
  }
  if (digitalRead(BUTTON_PREV) == HIGH && flag[1] == 1) {      //Действия после отпускания кнопки:
    if (mode == 0) mode = modes;
    else {
      mode--;
    };                                            // Переключение режима
    flag[1] = 0;
  }                                              //Сброс флага кнопки





  //Действия при определённом режиме:
  if (mode >= 12 && mode <= 14) {
    disp.displayByte(0, 0, 0, 0);
    disp.point(0);
  }

  if (mode == 0 && !sun) {
    setcolorall(0, 0, 0);  //выключен
    disp.displayByte(0, 0, 0, 0);
    disp.point(0);

  }
  else if (mode != 0 && mode <= 8) {
    setcolorall(colors[0][mode - 1], colors[1][mode - 1], bright);  //статичный свет

  }
  else if (mode == 9) fade(); //переливающийся свет

  else if (mode == 10) rainbow(1); //радуга по часовой стрелке

  else if (mode == 11) rainbow(-1); //радуга против часовой стрелки

  else if (mode == 12) colormusic(-1); //цветомузыка с переливом по часовой стрелке

  else if (mode == 13) colormusic(0); //цветомузыка

  else if (mode == 14) colormusic(1); //цветомузыка с переливом против часовой стрелки

  else if (mode == 15) gettime(); //время

  else if (mode == 16) {
    int napr = round(constrain(map(readVcc(), 3000, 4200, 0, 100), 0, 100));
    circle((12 * napr) / 100, napr, bright);
    //setcolorall(napr, 255, 100);
    disp.displayInt(napr);
    disp.point(0);
  }  //заряд аккумулятора



#if DEBUG
  Serial.println(mode);
#endif

  //Действия при подачи команды с пульта

  int cmd = ir_cmd();
  switch (cmd) {

    case 0:
      bright = constrain(bright + 15, 0, 255); //Увеличение яркости
      break;

    case 1:
      bright = constrain(bright - 15, 0, 255); //Уменьшение яркости
      break;

    case 2:
      if (mode != 0 && mode != 15) {
        last_mode = mode;  //Выключение ночника (режим 0)
        mode = 0;
      }
      else {
        three_beep(500, 750, 500, 500);
        full_sleep(mode == 15); // режим энергосбережения
      }
      break;

    case 3:
      mode = last_mode; //Включение ночника (возврат в недавний режим)
      break;

    // Изменеие оттенка и насыщенности
    case 4:
      if (mode > 0 && mode < 9) colors[0][mode - 1] = constrain(colors[0][mode - 1] - 16, 0, 255);
      else if (mode==15){
        bright_screen = constrain(bright_screen - 1, 0, 7);  
        disp.brightness(bright_screen);
      }
      break;

    case 5:
      if (mode > 0 && mode < 9) colors[0][mode - 1] = constrain(colors[0][mode - 1] + 16, 0, 255);
      else if (mode==15) {
        bright_screen = constrain(bright_screen + 1, 0, 7);  //Уменьшение яркости дисплея в режиме часов
        disp.brightness(bright_screen);
      }
      break;

    case 6:
      if (mode > 0 && mode < 9) colors[1][mode - 1] = constrain(colors[1][mode - 1] - 16, 0, 255);
      else wait += 200;
      break;

    case 7:
      if (mode > 0 && mode < 9) colors[1][mode - 1] = constrain(colors[1][mode - 1] + 16, 0, 255);

      else wait -= 200;

      break;

    // Переключение режима
    case 8:
      if (mode == 0) mode = modes;
      else mode--;
      break;

    case 9:
      if (mode == modes)  mode = 0;
      else mode++;
      break;

    // Цветомузыка и сброс шумов в режиме цветомузыки
    case 10:
      if (mode != 13) mode = 13;

      else denoise();
      break;

    // Выключение звука или отключение рассвета
    case 11:
      if (!sun) mute = !mute;

      else {
        alarm = false;
        sun = false;
      }
      break;

    // Сохранение настроек, установка времени в режиме часов, установка цветов в режиме цветомузыки
    case 22:
      if (mode != 15 && mode != 13) saveopt();

      else if (mode == 15) timeset();

      else set_cmcolors();
      break;

    // Часы, установка и сброс будильника в режиме часов
    case 23:
      if (mode != 15) mode = 15;

      else if (alarm == false) setalarm();

      else if (alarm) {
        alarm = false;
        EEPROM.write(20, alarm);
      }
      break;

    // Установка типа часов
    default:
      if (mode == 15 && cmd >= 12 && cmd <= 14) {
        disp.clear();
        type_clock = cmd - 12;
      }


  }
  if (cmd >= 4 && cmd <= 9 && mode >= 1 && mode <= 8) display_timer(millis() / 1000, (colors[0][mode - 1] / 16 + 1) * 100 + (colors[1][mode - 1] / 16), 1);
  if (cmd >= 6 && cmd <= 7 && mode >= 9 && mode <= 11) display_timer(millis() / 1000, 50 - (wait / 200), 0);



  wait = constrain(wait, 200, 10000);

#if DEBUG
  Serial.print("wait: ");
  Serial.println(wait);

#endif

  if (alarm) {
    check_alarm();
  }
  if (timer_color && mode >= 1 && mode <= 11) {
    display_timer(0, -1, 2);
  }
}

//Считывание команды с пульта
int ir_cmd() {

  if (IRLremote.available()) {
    //Считывание номера команды
    auto data = IRLremote.read();
    auto cmd = data.command;

    if (data.address != 65535) {
      if (mute == false) {
        tone(BEEP_PIN, 500, 100);
      }
      return cmd;
    }
    else return -1;
  }

  else return -1;
}

//---------------------------------------Сохранение настроек-----------------------------------------------
void saveopt() {
  for (byte i = 0; i < 8; i++) {
    EEPROM.write(i, colors[0][i]);
    EEPROM.write(i + 8, colors[1][i]);
  }
  EEPROM.write(16, bright);
  EEPROM.write(17, mute);
  EEPROM.write(50, bright_screen);
}


void full_sleep(bool cl) {
  digitalWrite(RTC_VCC, LOW);
  digitalWrite(IR_VCC, LOW);

  EEPROM.write(50, bright_screen);

  if (bright>0){
  EEPROM.write(16, bright);
  bright = 1;
  }
  
  disp.brightness(0);

  while ((digitalRead(BUTTON_PREV) == HIGH || digitalRead(BUTTON_NEXT) == HIGH) && !sun) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    digitalWrite(RTC_VCC, HIGH);
    if (cl) gettime();
    disp.point(1);
    if (alarm) check_alarm();
    digitalWrite(RTC_VCC, LOW);

  }
  if (!mute && !sun) {
    three_beep(1000, 750, 1000, 500);
  }

  if (bright>0){
  bright = EEPROM.read(16);
  }
  bright_screen = EEPROM.read(50);
  digitalWrite(RTC_VCC, HIGH);
  digitalWrite(IR_VCC, HIGH);
}


//------------------------------------------Удаление шумов------------------------------------------------
void denoise() {
  low = 0;

  for (byte i = 0; i < 100; i++) {
    for (byte i = 0 ; i < FHT_N ; i++) {
      int sample = analogRead(MIC_PIN);
      fht_input[i] = sample;
    }

    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();

    for (byte j = 2; j < FHT_N / 2; j++) {
      if (fht_log_out[j] > low) {
        low = fht_log_out[j];
      }
    }
  }
}


float my_vcc_const = 1.081;
long readVcc() {

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  result = my_vcc_const * 1023 * 1000 / result; // расчёт реального VCC
  return result; // возвращает VCC
}
