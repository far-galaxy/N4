//----------------------------------Функция цветомузыки-------------------------------------------

byte color[12];
byte last_color[] = {0,0,0,0,0,0,0,0,0,0,0,0};
byte color_f[] = {0,0,0,0,0,0,0,0,0,0,0,0};
byte max_val_f = 50;
float k_v = 0.7;
float k_c = 0.7;



void colormusic(){
  ADCSRA |= 1 << ADEN; // Включаем АЦП
  //Считывание спектра
  for (byte i = 0 ; i < FHT_N ; i++) {
    int sample = analogRead(MIC_PIN);
    fht_input[i] = sample; }
    
  fht_window();  // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run();     // process the data in the fht
  fht_mag_log(); // take the output of the fht
 
  color[0]=fht_log_out[2];
  color[1]=fht_log_out[3];
  color[2]=fht_log_out[4];
  
  color[3]=0;
  for (byte i = 5; i < 6; i++) {  if (fht_log_out[i] > color[3]) { color[3] = fht_log_out[i]; }}

  color[4]=0;
  for (byte i = 6; i < 7; i++) {  if (fht_log_out[i] > color[4]) { color[4] = fht_log_out[i]; }}

  color[5]=0;
  for (byte i = 7; i < 8; i++) {  if (fht_log_out[i] > color[5]) { color[5] = fht_log_out[i]; }}

  color[6]=0;
  for (byte i = 8; i < 9; i++) {  if (fht_log_out[i] > color[6]) { color[6] = fht_log_out[i]; }}
  
  color[7]=0;
  for (byte i = 9; i < 10; i++) {  if (fht_log_out[i] > color[7]) { color[7] = fht_log_out[i]; }}

  color[8]=0;
  for (byte i = 10; i < 11; i++) {  if (fht_log_out[i] > color[8]) { color[8] = fht_log_out[i]; }}

  color[9]=0;
  for (byte i = 11; i < 15; i++) {  if (fht_log_out[i] > color[9]) { color[9] = fht_log_out[i]; }}

  color[10]=0;
  for (byte i = 15; i < 21; i++) {  if (fht_log_out[i] > color[10]) { color[10] = fht_log_out[i]; }}

  color[11]=0;
  for (byte i = 21; i < 32; i++) {  if (fht_log_out[i] > color[11]) { color[11] = fht_log_out[i]; }}

  //Преобразование
  byte max_val = low+10;
  for (byte i=2; i<12; i++){
  //Serial.print(color[i]); Serial.print(" ");
  if(color[i]>max_val) {max_val = color[i];}}
 // Serial.println(" ");

  //disp.integer(max_val);

  
  for (byte i=0; i<12; i++){
    color_f[i] = int(color[i] * k_c + color_f[i] * (1 - k_c));
    color[i] = constrain(map(color_f[i],low,max_val,0,255),0,255);
    color[i] = constrain(color[i], last_color[i]-fade_out_time, 255);
    last_color[i] = color[i];
  }
  
  showCM();
  
  delay(10);
  
}

//----------------------------------Вывод спектра-------------------------------------------

void showCM(){
  for (byte i=0; i<LED_NUM; i++) leds[i] = CHSV(colors_music[0][i],colors_music[1][i],color[i]);
  led_show();  
}

void set_cmcolors(){
  threeBeep(750,1000,500,500);

  byte num_c = 0;
  boolean end_set = false;
  
  while(!end_set){
  byte led = num_c;
  FastLED.clear();
  leds[led] = CHSV(colors_music[0][led],colors_music[1][led],bright_led);
  led_show();
  
  //Действия при подачи команды с пульта
  int cmd = irInt(readIR());

  if (cmd != IR_CMD && sound) tone(BEEP_PIN,1000,100);
      switch(cmd){
        
        case 4:
        colors_music[0][led]=constrain(colors_music[0][led]-16,0,255);
        break;

        case 5:
        colors_music[0][led]=constrain(colors_music[0][led]+16,0,255);
        break;

        case 6:
        colors_music[1][led]=constrain(colors_music[1][led]-16,0,255);
        break;
        
        case 7:
        colors_music[1][led]=constrain(colors_music[1][led]+16,0,255);
        break;

        case 8:
        if (num_c == 0) {num_c = LED_NUM-1;}  
        else {num_c--;}
        break;

        case 9:
        if (num_c == LED_NUM-1) {num_c=0;}  
        else {num_c++;}
        break;

        case 22:
        for(byte i=0; i<11; i++){
          EEPROM.write(HUE_M+i,colors_music[0][i]);
          EEPROM.write(SAT_M+i,colors_music[1][i]);
        }
        threeBeep(500,1000,750,500);
        end_set = true; 
        break;


      }
    
  
}
}

void denoise() {
  low = 0;
  led_off();

  for (byte i = 0; i < 100; i++) {
    for (byte i = 0 ; i < FHT_N ; i++) {
      int sample = analogRead(MIC_PIN);
      fht_input[i] = sample;
    }

    fht_window();
    fht_reorder();
    fht_run();
    fht_mag_log();

    for (byte j = 4; j < FHT_N / 2; j++) {
      if (fht_log_out[j] > low) {
        low = fht_log_out[j];
      }
    }

    
  }

}
