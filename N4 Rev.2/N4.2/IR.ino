
//Перевод команды пульта в порядковый номер
int irInt(unsigned long hexIr){
  int cmd = IR_CMD;
  for (byte i=0; i<IR_CMD; i++) if (ir_data[i]==hexIr) cmd = i;
  return cmd;
}

//Чтение команд с пульта
unsigned long readIR(){
  //Если пришли данные с пульта
  if (IRLremote.available())
  {
    //Считывание номера команды
    auto data = IRLremote.read();
    auto cmd = data.command;

    //Сброс таймера
    if (!ir_rec){
      ir_rec = true;
      ir_timer = millis();
    }

    //Проверка команды
    if (cmd!=292984781){ return cmd;}
    else return 0;
  }
  else return 0;
}

void irRead(){
  //Если была принята команда с пульта, через 30 секунд бездейстия сохранить настройки
  if (ir_rec && (millis() > (ir_timer + SAVE_TIME*1000))){
    ir_rec = false;
    saveEEPROM();  
  }

  
  
  int cmd = irInt(readIR());

  if (cmd != IR_CMD && sound) tone(BEEP_PIN,1000,100);

  #if DEBUG
  if(cmd!=IR_CMD) Serial.println(cmd);
  #endif
  
  
  switch(cmd){
    case 0:
      if (mode != OPT_MODE) bright_led = constrain(bright_led + 16, 0, 255); //Увеличение яркости
      else {
        if (opt_val == 0) opt_val = 0; 
        else opt_val--; 
      }
      
    break;

    case 1:
      if (mode != OPT_MODE) bright_led = constrain(bright_led - 16, 0, 255); //Увеличение яркости
      else {
        if (opt_val >= opt_count[submode]) opt_val = opt_count[submode]; 
        else opt_val++;
      }
    break;

    case 2:
    if (mode != OFF_MODE) {
      last_mode = mode;  
      mode = OFF_MODE;
      displayTime(nameModes[0], 0);
    }
    break;

    case 3:
      mode = last_mode;
      displayTime(nameModes[mode], 0);
    break;

    // Изменеие оттенка и насыщенности
    case 4:
      if (mode == COLOR_MODE) colors[0][submode] = constrain(colors[0][submode] - 16, 0, 255);
      else if (mode == CLOCK_MODE){
        bright_display = constrain(bright_display - 1, 0, 7);  
        disp.bright(bright_display);
      }
    break;

    case 5:
      if (mode == COLOR_MODE) colors[0][submode] = constrain(colors[0][submode] + 16, 0, 255);
      else if (mode == CLOCK_MODE) {
        bright_display = constrain(bright_display + 1, 0, 7);  //Уменьшение яркости дисплея в режиме часов
        disp.bright(bright_display);
      }
    break;

    case 6:
      if (mode == COLOR_MODE) colors[1][submode] = constrain(colors[1][submode] - 16, 0, 255);
      else if (mode == GRADIENT_MODE || mode == RAINBOW_MODE){
        transfusion = constrain(transfusion - 5, 5, 255);
        displayTimeWI(_S, transfusion);
      }
    break;

    case 7:
      if (mode == COLOR_MODE) colors[1][submode] = constrain(colors[1][submode] + 16, 0, 255);
      else if (mode == GRADIENT_MODE || mode == RAINBOW_MODE){
        transfusion = constrain(transfusion + 5, 5, 255);
        displayTimeWI(_S, transfusion);
      }
    break;

    // Переключение режима
    case 8:
      if (mode == 0) mode = MODES-1; 
      else mode--;
      submode = 0;
      displayTime(nameModes[mode], 0);
    break;

    case 9:
      mode = (mode + 1) % (MODES);
      submode = 0;
      displayTime(nameModes[mode], 0);
    break;

    // Цветомузыка и сброс шумов в режиме цветомузыки
    case 10:
      if (mode != MUSIC_MODE) {
        mode = MUSIC_MODE;
        displayTime(nameModes[mode], 0);
      }
      else denoise();
    break;

    // Выключение звука или отключение рассвета
    case 11:
      if (!sun) sound = !sound;
      if (sun) {
        alarm = false;
        sun = false;
        mode = OFF_MODE;
        disp.clear();
      }
    break;   

    case 22:
      if (mode != CLOCK_MODE && mode != MUSIC_MODE) saveEEPROM();

      else if (mode == CLOCK_MODE) timeSet();

      else set_cmcolors();
      break;

   case 23:
      if (mode != CLOCK_MODE) mode = CLOCK_MODE;

      else if (alarm == false) setAlarm();

      else if (alarm) {
        alarm = false;
        EEPROM.write(ALARM, alarm);
      }
      break;
  }

  if (cmd>=12 && cmd <=21){
        submode = constrain(cmd-12,0,submodes[mode]-1);
        dsp_data[3] = disp.intNum(submode);
        displayTime(dsp_data,0);
      }
}

uint8_t nIR[24][4] = {
{0,_b,0,_P},
{0,_b,0,_dash},
{0,_O,_F,_F},
{0, 0,_O,_N},
{0,_H,0,_dash},
{0,_H,0,_P},
{0,_S,0,_dash},
{0,_S,0,_P},
{0,_N,0,_dash},
{0,_N,0,_P},
{0,_N,_U,_S},
{0,_S,_N,_d},
{0, 0, 0,_0},
{0, 0, 0,_1},
{0, 0, 0,_2},
{0, 0, 0,_3},
{0, 0, 0,_4},
{0, 0, 0,_5},
{0, 0, 0,_6},
{0, 0, 0,_7},
{0, 0, 0,_8},
{0, 0, 0,_9},
{0,_S,_A,_U},
{0,_C,_L,_C}};

//Установка данных пульта
void setIR(){
  
  disp.point(1);
  disp.display(_I,_r,_S,_t);
  fill(CRGB(0,0,0)); 
  if (sound) threeBeep(750, 1000, 500, 500);
  delay(1000);
  disp.point(0);

  for (byte i=0; i<IR_CMD; i++) ir_data[i] = 0;
  
  for (byte i=0; i<IR_CMD; i++){
    unsigned long cmd = readIR();     
    disp.display(nIR[i]);
    while (cmd == 0 || checkIR(cmd) == false){cmd = readIR();}
    if (sound) tone(BEEP_PIN,1000,100);

    #if DEBUG
      Serial.println(cmd, HEX); 
    #endif
    
    ir_data[i] = cmd;

    IRLremote.end(IR_PIN);  
    fill(CRGB(0,10,0));
    delay(300);
    fill(CRGB(0,0,0)); 
    delay(300);
    IRLremote.begin(IR_PIN);
  }
  disp.display(_I,_r,_O,_C);
  if (sound) threeBeep(750, 1000, 500, 500);
  delay(1000);
  
  for (byte i=0; i<IR_CMD; i++){
    EEPROM.put((i*4)+IR_DAT,ir_data[i]); 
    #if DEBUG
      Serial.print(ir_data[i]); 
      Serial.print(" ");
    #endif
    }
}

bool checkIR(unsigned long ir_cmd){
  bool ok = true;
  for (byte i=0; i<IR_CMD; i++){
    ok &= (ir_cmd != ir_data[i]);
  }
  return ok;
}
