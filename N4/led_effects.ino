//---------------------------------Функция переливающегося света----------------------------------------
byte fade_drag=0;

void fade(){

  for (int i=0; i<12; i++){leds[i] = CHSV(fade_drag,255,bright);}
  
  FastLED.show();
  fade_drag = (fade_drag+1)%255;
  delayMicroseconds(wait*3);
  
}  


//---------------------------------Функция радуги--------------------------------------------------------

void rainbow(int rotate){

  for (int i=0; i<12; i++){leds[i] = CHSV(fade_drag+(i*16),255,bright);}
  FastLED.show();
  fade_drag = (fade_drag+rotate)%255;
  delayMicroseconds(wait);
  
}  


//Функция отрисовки спектра
void setcolor_m(byte drag){
  
  for (int i=0; i<12; i++){leds[i] = CHSV(drag+colors_music[0][i],colors_music[1][i],color[i]);} 
  FastLED.show();

}

//Функция установки цвета светодиода
void setcolorall(byte h, byte s, byte v){
  
  for (int i=0; i<12; i++){leds[i] = CHSV(h,s,v);}  
  FastLED.show();

}

void clocks(byte h, byte m, byte s, byte br){

  s/=5;
  h%=12;
  m/=5;
  
  FastLED.clearData();
  leds[0] = CHSV(0,0,br);
  leds[h] = CHSV(0,255,br);
  leds[m] = CHSV(96,255,br);
  leds[s] = CHSV(160,255,br);
  FastLED.show();
   
}

void circle(byte k, byte c, byte br){
  FastLED.clearData();
  for(byte i=0; i<k; i++) leds[i] = CHSV(c,255,br);
  FastLED.show();
}
