//----------Инициализация----------------
void start(){

  #if DEBUG
  Serial.begin(9600);
  //printEEPROM();
  #endif

  //Увеличение частоты АЦП
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);
  //Снижение напряжения АЦП
  analogReference(INTERNAL);

  //Назначение пинов
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(RTC_VCC, OUTPUT);
  pinMode(IR_VCC, OUTPUT);
  //pinMode(CHG_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);


  // Включение часов и ИК
  digitalWrite(RTC_VCC, HIGH);
  digitalWrite(IR_VCC, HIGH);

  //Serial.println("Power");
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_NUM);  // запуск диодов
  IRLremote.begin(IR_PIN);                                // запуск ИК
  Clock.begin();                                          // запуск часов
  disp.clear();
  disp.bright(4);

  readEEPROM(); // считывание памяти

  if (digitalRead(BUTTON_PREV) == LOW) {
    setIR();           // вход в режим настройки пульта
  }

  // Очистка памяти
  if (digitalRead(BUTTON_NEXT) == LOW) {
    clearEEPROM();
    disp.display(_C,_L,_E,_r); delay(1000);
    }

  

  // Приветственное сообщение с выводом текущего заряда
  transit(CHSV(getCharge(),255,100), transit_type);
  disp.point(1);
  disp.display(_H,_I,_N,_4);

  clk = Clock.getTime();
  
  
  //Приветственный сигнал, если не выключен звук
  if (sound) {
    threeBeep(500, 750, 1000, 200);
    tone(BEEP_PIN, 1000, 300);
  }  
 delay(2000);
  
  transit(CRGB::Black, transit_type);
  disp.clear();
  disp.bright(bright_display);

  // Шумоподавление
  denoise();
  
  
}


// Действия с кнопками
void buttonRead(){

  //Кнопки удерживаются - переключение между основными режимами
  if(nextBut.holded()){
    mode = (mode + 1) % (MODES); 
    submode = 0;
    displayTime(nameModes[mode], 0);
    }
    
  if(prevBut.holded()){
    if (mode == 0) mode = MODES-1; 
    else mode--; 
    submode = 0;
    displayTime(nameModes[mode], 0);
    }
  
  //Кнопки щёлкаются - переключение между подрежимами
  if(nextBut.clicked()){
    submode = (submode + 1) % (submodes[mode]);
    dsp_data[3] = disp.intNum(submode);
    displayTime(dsp_data,0);
    }
  
  if(prevBut.clicked()){
    if (submode == 0) submode = submodes[mode]-1; 
    else submode--;
    dsp_data[3] = disp.intNum(submode);
    displayTime(dsp_data,0);
    }
}



// Вывод данных на дисплей с таймером
void displayTime(uint8_t data[4], byte dot){
  displayed = true;
  display_timer = millis();
  disp.point(dot);
  disp.display(data);
}

// Вывод данных (символ и число до 999) на дисплей с таймером 
void displayTimeWI(uint8_t sym, int num){
  displayed = true;
  display_timer = millis();
  disp.point(0);  
  disp.integer(num);
  disp.display(0, sym);
}

// Обновление дисплея (если есть вывод с таймером)
void displayUpd(){
  if (displayed && (millis() > display_timer + display_time*1000)){
    disp.clear();
    displayed = false;
    }  
}

//Пищалка
void threeBeep(int f, int s,int r, int wt){
  tone(BEEP_PIN,f,500);
  delay(wt);
  tone(BEEP_PIN,s,500);
  delay(wt);
  tone(BEEP_PIN,r,500);
  delay(wt);
}

void Celebration(){
  if (clk.date == 1 && clk.mon == 1 && !played){
    uint16_t yr = clk.year; 
    disp.integer(yr);
    for (byte i = 0; i < 2; i++){
      PlayMusic("MLT150L8O4");
      fill(CHSV(0, 0, 100));
      PlayMusic("C#DL4EL2O5C#");
      fill(CHSV(160, 200, 100));
      PlayMusic("L8O4C#DL4EL2B");
      fill(CHSV(192, 200, 100));
      PlayMusic("L8C#D#L2F");
      fill(CHSV(128, 200, 100));
      PlayMusic("L2AL4G#F#G#F#L8EE ");
      fill(CHSV(64, 200, 100));
      PlayMusic("L2F#L8EL4E F#EL8DL4C#L8 L4C#L8DE EDC#L2<B");
    }
    disp.clear();
    played = true;
  }

  if (clk.date == HB_DAY && clk.mon == HB_MON && !played){
    fill(CHSV(64, 200, 100));
    disp.display(_H,_A,_P,_Y);
    PlayMusic("MNT150L8O4CCL4DCFL2EL8CCL4DCGL2FL8CCL4>C<AFEL2DL8A+A+");
    disp.display(_b,_d,_A,_Y);
    PlayMusic("L4AFGL2FL8CCL4DCFL2EL8CCL4DCGL2FL8CCL4>C<AFEL2DL8A+A+");
    disp.display(_A,_N,_N,_A);
    PlayMusic("L4AFGL2F");
    disp.clear();
    played = true;
  }
}

// Вывод заряда
void showCharge(){
  if (millis()-1000 > chg_timer){
    chg_timer = millis();
    byte charge = getCharge();
    //if (digitalRead(CHG_PIN) == LOW){
      disp.point(0);
      disp.integer(charge);
      circle((12 * charge) / 100, charge, 100);
    //}
   // else {
   //   disp.point(0);
   //   displayTimeWI(_C, charge);
   //   circle((12 * charge) / 100, CHG_COL, 100);
    //}
    
  }
}




//map с ограничением
#define constMap(val, old_low, old_up, low, up) constrain(map(val, old_low, old_up, low, up), low, up)

//Перевод напряжения в проценты
byte getCharge(){
  return constMap(readVcc(), LOW_VCC, HIGH_VCC, 0, 100);
}

//Считывание напряжения питания
float my_vcc_const = 1.081;
long readVcc() {
  ADCSRA |= 1 << ADEN; // Включаем АЦП
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  result = my_vcc_const * 1023 * 1000 / result; // расчёт реального VCC
  return result; // возвращает VCC
}
