/* Структура памяти
   0 - наличие данных
   1 - звук
   2 - яркость светодиодов
   3 - яркость дисплея
   4 - тип перехода
   5 - скорость перехода
   4 - состояние будильника
   5,6,7 - часы, минуты, рассвет будильника
   8-17 - оттенки света
   18-27 - насыщенность света
   28-123 - данные пульта
   124-135 - оттенки светомузыки
   136-147 - насыщенность светомузыки
 */



void readEEPROM(){
  //Если есть сохранённые данные
  if (EEPROM.read(EXIST) == EXIST_NUM){
    
    sound = EEPROM.read(SOUND);
    bright_led = EEPROM.read(LED); 
    bright_display = EEPROM.read(DISP);
    alarm = EEPROM.read(ALARM); 
    transit_type = EEPROM.read(TR_TYPE); 
    
    for (byte i=0; i<3; i++) alarm_time[i] = EEPROM.read(TIME+i); 
    for (byte i=0; i<10; i++) {colors[0][i] = EEPROM.read(HUE_C+i); colors[1][i] = EEPROM.read(SAT_C+i);}
    for (byte i=0; i<LED_NUM; i++) {colors_music[0][i] = EEPROM.read(HUE_M+i); colors_music[1][i] = EEPROM.read(SAT_M+i);}
    for (byte i=0; i<IR_CMD; i++) {EEPROM.get((i*4)+IR_DAT, ir_data[i]);}
  }
  
  //Если их нет
  else {
    //Установка обычных цветов по умолчанию
    colors[0][0] = 0; colors[1][0] = 0;
    for (byte i=0; i<COLOR_COUNT-1; i++)  {colors[0][i+1] = i*32; colors[1][i+1] = 255;}
    //Установка цветов цветомузыки по умолчанию
    for (byte i=0; i<LED_NUM; i++) {colors_music[0][i] = i*16; colors_music[1][i] = 255;}
    alarm = false;
    saveEEPROM();
  }
}

void saveEEPROM(){
  EEPROM.write(EXIST, EXIST_NUM);
  EEPROM.write(SOUND, sound);
  EEPROM.write(LED, bright_led);
  EEPROM.write(DISP, bright_display);
  EEPROM.write(TR_TYPE, transit_type);
  EEPROM.write(ALARM, alarm);
  for (byte i=0; i<2; i++) EEPROM.write(TIME+i, alarm_time[i]); 
  for (byte i=0; i<10; i++) {EEPROM.write(HUE_C+i, colors[0][i]); EEPROM.write(SAT_C+i, colors[1][i]);}
  for (byte i=0; i<IR_CMD; i++){EEPROM.put((i*4)+IR_DAT, ir_data[i]);}
  for (byte i=0; i<LED_NUM; i++) {EEPROM.write(HUE_M+i, colors_music[0][i]); EEPROM.write(SAT_M+i, colors_music[1][i]);}
}

void clearEEPROM(){
  for (unsigned int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

#if DEBUG
void printEEPROM(){
  for (int i=0; i<SAT_M+LED_NUM; i++){
    Serial.print(i);
    Serial.print(" ");
    Serial.println(EEPROM.read(i));
  }
}
#endif
