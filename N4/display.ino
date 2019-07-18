unsigned long last_timer = 0;
int last_val = 0;
byte last_dot = 0;
void display_timer(long timer, int val, byte dot){
  if (!timer_color or timer!=0) last_timer = timer;
  if (val!=-1) last_val = val;
  if (dot!=2) last_dot = dot;
  if (millis()/1000-5<last_timer) {
    disp.displayInt(last_val);
    disp.point(last_dot);
    timer_color = true;
  }
  else{ disp.displayByte(0,0,0,0); disp.point(0); ; timer_color = false;}
}
