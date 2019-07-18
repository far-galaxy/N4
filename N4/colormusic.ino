//----------------------------------Функция цветомузыки-------------------------------------------

byte color[12];
byte last_color[] = {0,0,0,0,0,0,0,0,0,0,0,0};
byte color_f[] = {0,0,0,0,0,0,0,0,0,0,0,0};



void colormusic(int fd){
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
  for (byte i = 5; i < 7; i++) {  if (fht_log_out[i] > color[3]) { color[3] = fht_log_out[i]; }}

  color[4]=0;
  for (byte i = 7; i < 9; i++) {  if (fht_log_out[i] > color[4]) { color[4] = fht_log_out[i]; }}

  color[5]=0;
  for (byte i = 9; i < 11; i++) {  if (fht_log_out[i] > color[5]) { color[5] = fht_log_out[i]; }}

  color[6]=0;
  for (byte i = 11; i < 13; i++) {  if (fht_log_out[i] > color[6]) { color[6] = fht_log_out[i]; }}
  
  color[7]=0;
  for (byte i = 13; i < 15; i++) {  if (fht_log_out[i] > color[7]) { color[7] = fht_log_out[i]; }}

  color[8]=0;
  for (byte i = 15; i < 17; i++) {  if (fht_log_out[i] > color[8]) { color[8] = fht_log_out[i]; }}

  color[9]=0;
  for (byte i = 17; i < 19; i++) {  if (fht_log_out[i] > color[9]) { color[9] = fht_log_out[i]; }}

  color[10]=0;
  for (byte i = 19; i < 21; i++) {  if (fht_log_out[i] > color[10]) { color[10] = fht_log_out[i]; }}

  color[11]=0;
  for (byte i = 21; i < 32; i++) {  if (fht_log_out[i] > color[11]) { color[11] = fht_log_out[i]; }}

  //Преобразование
  byte max_val=50;
  for (byte i=0; i<12; i++){
  if(color[i]>max_val) {max_val = color[i];}}

  max_val_f = int(max_val * k_v + max_val_f * (1 - k_v));

  max_val_f = constrain(max_val_f,low,255);
  
  for (byte i=0; i<12; i++){
  color_f[i] = color[i] * k_c + color_f[i] * (1 - k_c);
  color[i] = constrain(map(color_f[i],low,max_val_f,0,bright),0,255);
  color[i] = constrain(color[i],last_color[i]-fade_out_time,255);
  last_color[i] = color[i];
  }
  
  setcolor_m(mov);
  mov = mov+((max_val/30)%255)*fd;
  
  delay(10);
  
}

//----------------------------------КОНЕЦ Функции цветомузыки-------------------------------------------


void set_cmcolors(){
  three_beep(750,1000,500,500);

  byte num_c = 0;
  boolean end_set = false;
  
  while(!end_set){
  byte led = num_c;
  FastLED.clear();
  leds[led] = CHSV(colors_music[0][led],colors_music[1][led],bright);
  FastLED.show();
  
  //Действия при подачи команды с пульта
  int cmd = ir_cmd();
  
      if(cmd!=-1 && mute==false){tone(BEEP_PIN,500,100);}
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
        if (num_c==0) {num_c=11;}  
        else {num_c--;}
        break;

        case 9:
        if (num_c==11) {num_c=0;}  
        else {num_c++;}
        break;

        case 22:
        for(byte i=0; i<11; i++){
          EEPROM.write(i+22,colors_music[0][i]);
          EEPROM.write(i+34,colors_music[1][i]);
        }
        three_beep(500,1000,750,500);
        end_set = true; 
        break;


      }
    
  
}
}
