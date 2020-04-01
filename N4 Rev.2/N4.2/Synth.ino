//Нота	0 1     2 3   	4 5 6  	  7     8      	9	10  	11
//Нота	C C#/D-	D D#/E-	E F F#/G- G	G#/A-	A	A#/B?	B
int freq[7][12] = {
 {65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123},                     //0 = Большая октава   
 {131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247},             //1 = Малая октава
 {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494},             //2 = 1-я октава
 {523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988},             //3 = 2-я октава
 {1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976}, //4 = 3-я октава
 {2093, 2218, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951}, //5 = 4-я октава
 {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902}, //6 = 5-я октава
};


int extractNumber(int& myNumber, char Muz[], int& curPosition)
{
    int digitsNumber=0;
    int curDigit=0;
    myNumber=0;
    do
    {
        if ((Muz[curPosition]> 47) && (Muz[curPosition]<58)) // Коды ASCII цифр '0' == 48 , "9' == 57
        { 
            curDigit=Muz[curPosition]-48;
            digitsNumber++;
            myNumber=myNumber*10+curDigit;
        }
        else
        {
        return digitsNumber;
        }     
        curPosition++;
    }while(Muz[curPosition]!= '\0');
    return digitsNumber;
}

int pointsCount(char Muz[], int& curPosition)
{
    int pointsNumber=0;
    do
    {
        if (Muz[curPosition]== '.')
        { 
            pointsNumber++;
        }
        else
        {
        return pointsNumber;
        }     
        curPosition++;
    }while(Muz[curPosition]!= '\0');
    return pointsNumber;
}

void PlayMusic(char Muz[])
{
  static int generalOktava;   
  int oktava;   
  static int tempo=120; // Задание темпа или четвертных нот, которые исполняются в минуту. n от 32 до 255. По умолчанию 120
  int Nota=0; 
  int  curPosition, curNota4;
  unsigned long currentNotaPauseDuration;
  unsigned long currentNotaDuration; 
  unsigned long  pauseDuration; 
  int takt=240000/tempo; 
  bool isNota; 
  bool isPause; 
  int pointsNum=0;
  float generalNotaMultipl=0.875;
  static float NotaLong;
  float curMultipl;
  float tempFlo;
  float curPause;
  unsigned long tempLong;
  int i=0;
  do 
  {
   isNota=false; 
   isPause=false; 
   oktava=generalOktava;
   switch(Muz[i]){
         case '\0':{
             return; 
             }
             break;
         case 'C':{
             Nota=0;
             isNota=true; 
             }
             break;
         case 'D':{
             Nota=2;
             isNota=true; 
             }
             break;
         case 'E':{
             Nota=4;
             isNota=true;      
             }
             break;
         case 'F':{
             Nota=5;
             isNota=true;     
             }
             break;
         case 'G':{
             Nota=7;
             isNota=true;         
             }
             break;
         case 'A':{
             Nota=9;
             isNota=true;
             }
             break;
         case 'B':{
             Nota=11;
             isNota=true; 
             }
             break;   
         case 'N':{// Nнота	Играет определенную ноту (0 - 84) в диапазоне семи октав (0 - пауза).     
                 curPosition=i+1;
                 if (extractNumber(curNota4, Muz, curPosition)){
                     i=curPosition-1;
                     if (curNota4){
                         curNota4--;          
                         oktava=curNota4 / 12;                
                         Nota=curNota4 % 12;
                         isNota=true; 
                     }        
                     else{
                         isPause=true;
                     }
                 }                
             }
             break;   
         case 'O':{ //Oоктава	Задает текущую октаву (0 - 6). 
             curPosition=i+1;
             if (extractNumber(oktava, Muz, curPosition)){
                 i=curPosition-1;
                 generalOktava=oktava; 
                 }
             }
             break;          
         case '>':{
             generalOktava++;
             }
             break;  
         case '<':{
             generalOktava--;
             }
             break;
         case 'M':{   
              switch(Muz[i+1]){
                 case 'N':{ //MN	Нормаль. Каждая нота звучит 7/8 времени, заданного в команде L
                     generalNotaMultipl=0.875; //  =7/8
                     i++;
                     }
                     break;
                 case 'L':{ //ML	Легато. Каждая нота звучит полный интервал времени, заданного в команде L
                     generalNotaMultipl=1.0;
                     i++;
                     }
                     break;                 
                 case 'S':{ //MS	Стаккато. Каждая нота звучит 3/4 времени, заданного в команде L
                     generalNotaMultipl=0.75;  // =3/4
                     i++;
                     }
                     break;                 
                 case 'F':{ //MF Режим непосредственного исполнения. Т.е. на время проигрывания ноты программа приостанавливается. Используется по умолчанию
                     i++;   //Сдвигаем точку чтения и ничего не делаем.
                     }
                     break;   

                 case 'B':{ //MB проигрывние в буффер
                     i++;   //Сдвигаем точку чтения и ничего не делаем.
                     }
                     break;   
               }
             }
             break;             
         case 'L':{ //Lразмер	Задает длительность каждой ноты (1 - 64). L1 - целая нота, L2 - 1/2 ноты и т.д.    
             curPosition=i+1;
             if (extractNumber(curNota4, Muz, curPosition)){
                 i=curPosition-1;
                 tempFlo=float(curNota4);
                 NotaLong=1/tempFlo;
                 }             
             }
             break;
         case 'T':{ //Tтемп	Задает темп исполнения в четвертях в минуту (32-255).По умолчанию 120
             curPosition=i+1;
             if (extractNumber(tempo, Muz, curPosition)){
                i=curPosition-1;
                takt=240000/tempo; // миллисекунд на 1 целую ноту. 240000= 60 сек * 1000 мсек/сек *4 четвертей в ноте
                }             
             }
             break;
         case 'P':{ //Pпауза	Задает паузу (1 - 64). P1 - пауза в целую ноту, P2 - пауза в 1/2 ноты и т.д.
             curPosition=i+1;
             if (extractNumber(curNota4, Muz, curPosition)){
                 tempFlo=float(curNota4);
                 curPause=1/tempFlo;
                 i=curPosition-1;
                 isPause=true;
                 }
             }
             break;              
         case ' ':{ //Есть в некоторых текстах. Вероятно это пауза длительностью в текущую ноту
             curPause= NotaLong;
             isPause=true;
             }
             break;  
    }          
    if (isNota){
      switch(Muz[i+1]){
         case '#':{ // диез
             Nota++;
             i++;
             }
             break;
         case '+':{ // диез
             Nota++;
             i++;
             }
             break;                 
         case '-':{ // бемоль
           Nota--;
           i++;
          }
          break;   
       }
       curPosition=i+1;
       if (extractNumber(curNota4, Muz, curPosition)){
         currentNotaDuration=takt/curNota4;
         i=curPosition-1;
         }
     }
     if (oktava<0) oktava=0; 
     if (oktava>6) oktava=6; 
     if (isNota || isPause){ 
       curPosition=i+1;
       pointsNum=pointsCount(Muz, curPosition);  
       if (pointsNum) i=curPosition-1;   
       curMultipl=1.0;                
       for (int j=1; j<=pointsNum; j++) {
          curMultipl= curMultipl * 1.5;
       }   
       currentNotaPauseDuration=(takt*NotaLong);
     } 
     if (isNota){
        curMultipl=curMultipl*generalNotaMultipl;
        currentNotaDuration= (currentNotaPauseDuration*curMultipl); 
        if (Nota<0) Nota=0; 
        if (Nota>11) Nota=11; 
        tempLong= freq[oktava][Nota];
        tone(BEEP_PIN,tempLong,currentNotaDuration);     
        delay(currentNotaPauseDuration);
     } 
     if (isPause){
         pauseDuration=takt*curPause*curMultipl;
         delay(pauseDuration);     
     } 
    i++;          
  } while (Muz[i]!= '\0');   
}


//Qb_PLAY ("MNT150L8O3AL16>C<AL8EL16AEL8CL16ECL4<AL16EA>C<ABABAG+");

 
