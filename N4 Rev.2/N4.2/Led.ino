//Включение светодиодов одним цветом
void fill(CRGB color){
  for (byte i=0; i<LED_NUM; i++)leds[i] = color;
  led_show();  
}

//Вывод массива на светодиоды
void led_show(){
  if (!IRLremote.receiving()){ // если на ИК приёмник не приходит сигнал (без этого НЕ РАБОТАЕТ!)
  FastLED.show(); }
}

//Переходы
void transit(CRGB color, byte type){
  bool old = true;
  for(byte i = 0; i < LED_NUM; i++) old &= (leds[i] == color);
  if (!old){   //проверка цвета (т.е. требуется ли переход)
    switch (type){
      case 0: fill(color); break;
      case 1: fadeTransition(color); break;
      case 2: clockTransition(color); break;

      default: fill(color); break;

      }
    }
}

//Переход "переливание"
void fadeTransition(CRGB color){
  
  int frames = (fade*100)/(DK);            //количество кадров перехода (время перехода/время кадра (1с / 50к/с))

  int dr[LED_NUM], dg[LED_NUM], db[LED_NUM];
  CRGB last[LED_NUM];
  for (byte i = 0; i < LED_NUM; i++){
    last[i] = leds[i];                     //предыдущий цвет
    dr[i] = (color.r - last[i].r) / frames;    //изменение цвета за кадр
    dg[i] = (color.g - last[i].g) / frames;
    db[i] = (color.b - last[i].b) / frames;
  }


  for (byte f = 0; f < frames; f++){
    for (byte i = 0; i < LED_NUM; i++){
    leds[i] = CRGB(last[i].r + int(dr[i]*f), last[i].g + int(dg[i]*f), last[i].b + int(db[i]*f));
    //fill(CRGB(last[i].r + int(dr[i]*i), last[i].g + int(dg[i]*i), last[i].b + int(db[i]*i)));
    }
    
    led_show();
    delay(DK);
  }
  fill(color); //иногда бывает, что цвет не доходит до конца
}

//Переход "часы"
void clockTransition(CRGB color){
  for (byte i=0; i<LED_NUM; i++){
    leds[i] = color;
    led_show(); 
    delay(fade*100/LED_NUM);
  }
}


byte fade_drag=0;
void fadeEffect(bool cw){
  if (millis()%(transfusion*100/256) == 0){
  for (int i=0; i<12; i++){leds[i] = CHSV(fade_drag,255,bright_led);} 
  led_show();
  fade_drag = (fade_drag + (cw ? 1: -1))%255;
  }
}  


//---------------------------------Функция радуги--------------------------------------------------------

void rainbowEffect(bool cw){
  if (millis()%(transfusion*100/256) == 0){
  for (int i=0; i<12; i++){leds[i] = CHSV(fade_drag+(i*16),255,bright_led);}
  led_show();
  fade_drag = (fade_drag + (cw ? 1: -1))%255;
  }
  
}  

void moon(byte phase, CRGB color){
  FastLED.clearData();
  switch(phase){
    case 0: led_off(); break;
    case 1: 
      leds[3] = color;
      led_show();
    break;

    case 2:
      for (byte i = 2; i <= 4; i++) leds[i] = color;
      led_show();
    break;
    
    case 3:
      for (byte i = 1; i <= 5; i++) leds[i] = color;
      led_show();
    break;
    
    case 4:
      for (byte i = 0; i <= 6; i++) leds[i] = color;
      led_show();
    break;

    case 5:
      for (byte i = 0; i <= 7; i++) leds[i] = color;
      leds[LED_NUM-1] = color;
      led_show();
    break;

    case 6:
      for (byte i = 0; i <= 8; i++) leds[i] = color;
      leds[LED_NUM-1] = color;
      leds[LED_NUM-2] = color;
      led_show();
    break;

    case 7:
      for (byte i = 0; i <= LED_NUM; i++) leds[i] = color;
      led_show();
    break;

    case 8:
      for (byte i = 4; i <= LED_NUM; i++) leds[i] = color;
      leds[0] = color;
      leds[1] = color;
      leds[2] = color;
      led_show();
    break;

    case 9:
      for (byte i = 5; i <= LED_NUM; i++) leds[i] = color;
      leds[0] = color;
      leds[1] = color;
      led_show();
    break;

    case 10:
      for (byte i = 6; i <= LED_NUM; i++) leds[i] = color;
      leds[0] = color;
      led_show();
    break;

    case 11:
      for (byte i = 7; i <= LED_NUM-1; i++) leds[i] = color;
      led_show();
    break;

    case 12:
      for (byte i = 9; i <= LED_NUM-2; i++) leds[i] = color;
      led_show();
    break;

    case 13:
      leds[9] = color;
      led_show();
    break;

    case 14: led_off(); break;
  }
}


void circle(byte k, byte c, byte br){
  FastLED.clearData();
  for(byte i=0; i<k; i++) leds[i] = CHSV(c,255,br);
  led_show();
}

void led_off(){
  fill(CRGB::Black);
}
