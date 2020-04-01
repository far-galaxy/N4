
void clockMode(){
  //if (alarm) digitalWrite(LED_BUILTIN, HIGH);
  //else digitalWrite(LED_BUILTIN, LOW);

  
  
  if (submode == 0){  
    byte h = byte(clk.hour);  
    byte m = byte(clk.min);
    byte s = byte(clk.sec);
  
    disp.clock(h, m, millis()/500%2);
    clockLed(h, m, s, alarm, bright_led);
  }

  if (submode == 1){  
    byte d = clk.date;  
    byte m = clk.mon;
  
    disp.clock(d, m, 1);
    moon(MoonPhase(clk.year, m, d), CHSV(0, 0, bright_led));
    //clockLed(h,m,s,bright_led);
  }

  if (submode == 2){  
    uint16_t yr = clk.year;  
    disp.point(0);
    disp.integer(yr);
    led_off();
  }
}

void clockLed(byte h, byte m, byte s, bool alrm, byte br){

  s/=5;
  h%=12;
  m/=5;
  
  FastLED.clearData();
  leds[0] = CHSV(64, 255*alrm, br);
  leds[h].r = br;
  leds[m].g = br;
  leds[s].b = br;
  led_show();
 
}

void timeSet(){
  disp.clear();
  delay(50);
  if (sound) threeBeep(1000,750,500,500);
  noTone(BEEP_PIN);
  disp.point(1);
  byte tme[]={0,0, 0,0};
  byte i=0;
  if (submode == 2){
    i = 2;
    disp.display(_2, _0, 0, 0);
    disp.point(0);
  }
  while (i<=3){
     int cmd = irInt(readIR());
       if (cmd>=12 && cmd<=21){
        
         #if DEBUG
         Serial.print(cmd-12);
         #endif
         
         tme[i]=cmd-12;
         if (sound) tone(BEEP_PIN,1000,100);
         delay(100);

         disp.display(i, disp.intNum(tme[i]));
         i++;
       }
       else {noTone(BEEP_PIN);}
       
  }
  
  if (submode == 0) Clock.setTime(tme[0]*10+tme[1], tme[2]*10+tme[3], 0);     
  if (submode == 1) Clock.setDate(tme[0]*10+tme[1], tme[2]*10+tme[3], clk.year);
  if (submode == 2) Clock.setDate(clk.date, clk.mon, 2000+tme[2]*10+tme[3]);
  delay(500);
  
 

  if (sound) threeBeep(1000,750,500,500);
  disp.clear();
}

void setAlarm(){
  alarm=true;
  EEPROM.write(ALARM,true);
  disp.clear();
  delay(50);
  if (sound) threeBeep(500,750,1000,500);
  noTone(BEEP_PIN);
  disp.point(1);
  
  byte alrm[]={0,0,0,0,0};
  byte i=0;
  while (i<=4){
     if (i==4) {disp.display(_S,_U,_N, _empty); disp.point(0);}

     int cmd = irInt(readIR());
       if (cmd>=12 && cmd<=21){
         
         alrm[i]=cmd-12;
         if (sound) tone(BEEP_PIN,1000,100);
         delay(100);

        
         if (i<4) disp.display(i, disp.intNum(alrm[i]));
         else disp.display(3, disp.intNum(alrm[i]));
         i++;
       }
       else {noTone(BEEP_PIN);}
       
  }
  delay(500);
  alarm_time[0]=constrain(alrm[0]*10+alrm[1],0,23);
  alarm_time[1]=constrain((alrm[2]*10+alrm[3]),0,59);
  alarm_time[2]=alrm[4]*10;

  EEPROM.write(TIME,alarm_time[0]);
  EEPROM.write(TIME+1,alarm_time[1]);
  EEPROM.write(TIME+2,alarm_time[2]);
  
  
  if (sound) threeBeep(500,750,1000,500);
  disp.clear();
}



void checkAlarm(){
  clk = Clock.getTime();
  
  int h = clk.hour; 
  int m = int(clk.min);
  int s = int(clk.sec);

  int delta = (alarm_time[0]*60+alarm_time[1])*60-(h*3600+m*60+s);
  float sunrise = (255.0/(alarm_time[2]*60))*(alarm_time[2]*60-delta);

  #if DEBUG
  Serial.print(h); Serial.print(" ");
  Serial.print(m); Serial.print(" ");
  Serial.print(s); Serial.println(" ");

  Serial.print(alarm_time[0]); Serial.print(" ");
  Serial.println(alarm_time[1]); 
  Serial.println(delta);
  Serial.println(sunrise);
  #endif

  if(delta<=alarm_time[2]*60 && delta>0 && alarm_time[2]!=0){
    mode=0;
    fill(CHSV(16,255-int(sunrise),int(sunrise)));
    disp.clock(h, m, millis()/500%2);
    alarm_rst=false;
    sun=true;
  }
  
  if(alarm_time[0]==h && alarm_time[1]==m && !alarm_rst){
    if (alarm_time[2]!=0) fill(CHSV(0,0,255));
    while(digitalRead(BUTTON_NEXT)==HIGH || digitalRead(BUTTON_PREV)==HIGH){
      tone(BEEP_PIN,2000,1000);
      delay(2000);
    }
    if(digitalRead(BUTTON_NEXT)==LOW || digitalRead(BUTTON_PREV)==LOW){
      alarm_rst=true; 
      sun=false; 
      tone(BEEP_PIN,1000,1000);
      delay(2000);
      mode = OFF_MODE;
      disp.clear();
      }
  }
}

byte MoonPhase(uint16_t yer, uint8_t m, uint8_t da){
  long d;
  float p;
  
  d = DayNumber(yer, m, da) - DayNumber(2000,1,6);
  p = d/29.530588853; 
  d = p;
  p -= d; 
  return byte(p*14);

}


long DayNumber(uint16_t yer, uint8_t m, uint8_t d){
  m = (m + 9) % 12;
  yer = yer - m/10;
  return 365*yer + yer/4 - yer/100 + yer/400 + (m*306 + 5)/10 + d - 1 ;
}
