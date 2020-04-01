byte num_color = 0;
bool enter_opt = false;

byte demo_color = 0;
void optMode(){
  if (submode == 1){
    transit_type = opt_val;
    transit(CHSV(colors[0][demo_color], colors[1][demo_color], bright_led), transit_type);
    demo_color = demo_color + 1 % 10;
  }
}

void setOptions(){
  switch (submode){
    case 0:
      opt_val = 0;
      enter_opt = false;
      led_off();
    break;
    
    case 1:
      if (!enter_opt){
        enter_opt = true;
        opt_val = transit_type;
      }
      disp.display(_F,_A,_d,_E);
      transit(CHSV(num_color,255,150), opt_val);
      transit_type = opt_val;
      num_color = num_color+32;
      delay(200);
    break;

    case 2:
      if (!enter_opt){
          enter_opt = true;
          opt_val = transfusion/10;
        }
  
      disp.display(_t,_i,_N,_E);
      transit(CHSV(num_color,255,150), transit_type);
      transfusion = opt_val*10;
      num_color = num_color+32;
      delay(200);
  }
  
}
     
