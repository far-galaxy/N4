//SCL - A5
//SDA - A4


//----------------------------------------Считывание времени---------------------------------------------------------

byte last_h=0;
byte last_m=0;
void gettime(){
  
  if (alarm) digitalWrite(13,HIGH);
  else digitalWrite(13,LOW);
  
  t = Clock.getTime();

  
  int h = int(t.hour);  
  int m = int(t.min);
  int s = int(t.sec);
  

  
  if (type_clock==0) disp.displayClock(h, m);
  if (type_clock==1) disp.displayClockTwist(h, m, 20);
  if (type_clock==2) disp.displayClockScroll(h, m, 20);

  
  disp.point(millis()/500%2);
  clocks(h,m,s,bright);  

  #if DEBUG
  Serial.print(Clock.getDateStr());
  Serial.println(Clock.getTimeStr());
  #endif

}


//----------------------------------Установка будильника-------------------------------------------------------------
void setalarm(){
  alarm=true;
  EEPROM.write(20,alarm);
  disp.clear();
  delay(50);
  three_beep(500,750,1000,500);
  noTone(BEEP_PIN);
  disp.point(1);
  
  byte alrm[]={0,0,0,0,0};
  byte i=0;
  while (i<=4){
     if (i==4) {disp.displayByte(_S,_U,_N, _empty); disp.point(0);}

     int cmd = ir_cmd();
       if (cmd>=12 && cmd<=21){
        
         #if DEBUG
         Serial.print((cmd-11)%10);
         #endif
         
         alrm[i]=(cmd-11)%10;
         tone(BEEP_PIN,1000,100);
         delay(100);

        
         if (i<4) disp.display(i, alrm[i]);
         else disp.display(3, alrm[i]);
         i++;
       }
       else {noTone(BEEP_PIN);}
       
  }
  delay(500);
  alarm_time[0]=constrain(alrm[0]*10+alrm[1],0,23);
  alarm_time[1]=constrain((alrm[2]*10+alrm[3]),0,59);
  alarm_time[2]=alrm[4]*10;

  #if DEBUG
  Serial.println(" ");
  Serial.print(alarm_time[0]);
  Serial.print(" ");
  Serial.println(alarm_time[1]);
  Serial.println(alarm_time[2]);
  #endif
  EEPROM.write(18,alarm_time[0]);
  EEPROM.write(19,alarm_time[1]);
  EEPROM.write(21,alarm_time[2]);
  
  three_beep(500,750,1000,500);
  disp.clear();
  disp.point(0);
 
}

//-----------------------------------------Проверка времени---------------------------------------------------------------------
boolean rst=false;

void check_alarm(){
  t = Clock.getTime();
  

  int h = t.hour; 
  int m = int(t.min);
  int s = int(t.sec);

  int delta = (alarm_time[0]*60+alarm_time[1])*60-(h*3600+m*60+s);
  float sunrise = 255.0/(alarm_time[2]*60)*(alarm_time[2]*60-delta);

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
    setcolorall(64,255-int(sunrise),int(sunrise));
    disp.displayClock(h, m);
    disp.point(1);
    rst=false;
    sun=true;
  }
  
  if(alarm_time[0]==h && alarm_time[1]==m && !rst){
    setcolorall(0,0,255);
    while(digitalRead(BUTTON_NEXT)==HIGH || digitalRead(BUTTON_PREV)==HIGH){
      tone(BEEP_PIN,2500,1000);
      delay(2000);
    }
    if(digitalRead(BUTTON_NEXT)==LOW || digitalRead(BUTTON_PREV)==LOW){rst=true; sun=false; tone(BEEP_PIN,1500,1000);delay(2000);}
  }
}



void timeset(){
  disp.clear();
  delay(50);
  three_beep(1000,750,500,500);
  noTone(BEEP_PIN);
  disp.point(1);
  byte tme[]={0,0, 0,0, 0,0, 0,0, 0,0};
  byte i=0;
  while (i<=3){
     int cmd = ir_cmd();
       if (cmd>=12 && cmd<=21){
        
         #if DEBUG
         Serial.print((cmd-11)%10);
         #endif
         
         tme[i]=(cmd-11)%10;
         tone(BEEP_PIN,1000,100);
         delay(100);

         disp.display(i, tme[i]);
         i++;
       }
       else {noTone(BEEP_PIN);}
       
  }
  //Clock.setDOW();     
  Clock.setTime(tme[0]*10+tme[1], tme[2]*10+tme[3], 0);     
  //Clock.setDate(tme[4]*10+tme[5], tme[6]*10+tme[7], 2000+tme[8]*10+tme[9]);
  delay(500);
  
 

  three_beep(1000,750,500,500);
  disp.clear();
  disp.point(0);
}
