unsigned long str = millis();

void three_beep(int f, int s,int r, int wt){
  tone(BEEP_PIN,f,500);
  delay(wt);
  tone(BEEP_PIN,s,500);
  delay(wt);
  tone(BEEP_PIN,r,500);
  delay(wt);
}
