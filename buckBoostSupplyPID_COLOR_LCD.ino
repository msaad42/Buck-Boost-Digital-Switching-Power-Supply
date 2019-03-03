#include <UTFT.h>
#include <UTouch.h>


int state=1;


UTFT    myGLCD(ILI9327,38,39,40,41);
UTouch  myTouch( 6, 5, 9, 3, 2);

// Declare which fonts we will be using
extern uint8_t arial_italic[];
extern uint8_t Inconsola[];
extern uint8_t hallfetica_normal[];
//extern uint8_t SevenSegNumFont[];
//extern uint8_t SevenSegmentFull[];
extern uint8_t GroteskBold32x64[];
extern uint8_t nadianne[];
//extern uint8_t swiss721_outline[];
extern uint8_t arial_bold[];
//extern uint8_t DotMatrix_M_Slash[];

int x, y;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";

//*********************switching global********************************
#include <PID_v1.h>
//int buckPin=5;
int buckPin=4;
//int boostPin=6;
int boostPin=13;
int boostLimit=230;
int numReadings=10;

double vin=0;

//buckPID variables
double vref=8;
double vout, buckPWM;
int voutRealDisplayCount=60;
int vinDisplayCount=100;
boolean switchToBuck=false;

//boostPID variables
double boostPWM;


//Specify the links and initial tuning parameters
//PID buckPID(&vout, &buckPWM, &vref,2,5,1, DIRECT);
PID buckPID(&vout, &buckPWM, &vref,3,30,0.5, DIRECT);
//PID boostPID(&vout, &boostPWM, &vref,2,5,1, DIRECT);
PID boostPID(&vout, &boostPWM, &vref,3,20,0.5, DIRECT);

//*********************end switching global********************************

/*************************
 **   Custom functions   **
 *************************/
void drawSetVoltageButton(){
    myGLCD.setColor(204, 204, 153);
    myGLCD.setBackColor(204, 204, 153);
    myGLCD.fillRoundRect (220,120,360,184);//x1,y1,x2,y2
    myGLCD.setColor(255,255,255);
    //myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.setFont(arial_bold);
    myGLCD.print("Set VOUT",225 ,150);
}


void drawButtons()
{
  // Draw the upper row of buttons
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.printNumI(x+1, 27+(x*60), 27);
  }
  // Draw the center row of buttons
  for (x=0; x<6; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 70, 60+(x*60), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 70, 60+(x*60), 120);
    if (x<4)
      myGLCD.printNumI(x+6, 27+(x*60), 87);
  }
  myGLCD.print("0", 267, 87);
  myGLCD.print(".", 327, 87);
  // Draw the lower row of buttons
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 130, 150, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 150, 180);
  myGLCD.print("Clear", 40, 147);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (160, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (160, 130, 300, 180);
  myGLCD.print("Enter", 190, 147);
  myGLCD.setBackColor (0, 0, 0);
}

void updateStr(int val)
{
  if (stCurrentLen<20)
  {
    stCurrent[stCurrentLen]=val;
    stCurrent[stCurrentLen+1]='\0';
    stCurrentLen++;
    myGLCD.setColor(0, 0, 0);
    //myGLCD.print(stCurrent, CENTER, 224);
    myGLCD.print(stCurrent, CENTER, 200);
  }
  else
  {
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("BUFFER FULL!", CENTER, 192);
    delay(500);
    myGLCD.print("            ", CENTER, 192);
    delay(500);
    myGLCD.print("BUFFER FULL!", CENTER, 192);
    delay(500);
    myGLCD.print("            ", CENTER, 192);
    myGLCD.setColor(0, 255, 0);
  }
}

// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

/*************************
 **  Required functions  **
 *************************/
 
void mainScreenDraw()//state=1
{
  myGLCD.clrScr();
  //myGLCD.fillScr(134,237,38);light green
  myGLCD.fillScr(239,233,242);//light grey
  drawSetVoltageButton();
  
  myGLCD.setBackColor(239,233,242);//light grey
  myGLCD.setColor(210, 100, 40);//light red
  myGLCD.setFont(arial_bold);
  myGLCD.print("Buck Boost Chopper Supply",LEFT,220);
  
  myGLCD.setFont(arial_bold);
  myGLCD.setColor(255,0,0);// red
  myGLCD.print("VOUT set",40,20);//for 1st voltage
  
  myGLCD.setColor(0,255,0);//light green
  myGLCD.print("VOUT real",240,20);//for 2nd voltage
  
  myGLCD.setColor(0, 0, 255);//blue
  myGLCD.print("VIN",65,190);//for 3rd voltage
  
  //VOUT set
  myGLCD.setFont(GroteskBold32x64);
  myGLCD.setColor(255,0,0);// red
  myGLCD.printNumF(vref,1,20,40);//1 decimal place, x=40, y=40, 46 = ".", 3 characters, 48= "0" (filler)
  
  myGLCD.setColor(255,0,0);// red
  myGLCD.print("V",160,40);//for 1st voltage
  
  myGLCD.setColor(0,255,0);//light green
  myGLCD.print("V",360,40);//for 2nd voltage
  
  myGLCD.setColor(0, 0, 255);//blue
  myGLCD.print("V",160,120);//for 3rd voltage
  
  //display VIN
  myGLCD.setColor(0, 0, 255);//blue
  myGLCD.printNumF(vin,1,20,120);//1 decimal place, x=40, y=40, 46 = ".", 3 characters, 48= "0" (filler)
  
  state=2;
  
}//end mainScreenDraw()
  
void inputScreenDraw()//state=3
{
  myGLCD.clrScr();
  myGLCD.fillScr(239,233,242);
  myGLCD.setFont(nadianne);
  myGLCD.setBackColor(0, 0, 255);
  drawButtons();  
  //myGLCD.setFont(Inconsola);
  myGLCD.setBackColor(239,233,242);

  state=4;
  
}//end inputScreenDraw()

void regulate()
{
  if (Serial.available() > 0) {

    double vol = Serial.parseFloat();
    Serial.println(vol);
    vref=vol;
    }
    
  vout=0;
  for(int i=0;i<numReadings;i++){
  // read the input on analog pin 0:
  vout = vout + analogRead(A1);
                      }
  vout=vout/numReadings;
  vout=vout*0.0493;
  //Input=Input*0.0229;
  Serial.print(vout);
  Serial.println(" V");
  
  if(buckPWM<255 && boostPWM==0 || switchToBuck==true){
  switchToBuck=false;
  buckPID.Compute();
  analogWrite(buckPin,buckPWM);
                  }
  else{
  boostPID.Compute();
  //if(boostPWM < boostLimit)
   boostPWM = map(boostPWM, 0, 255, 0, 230);
   analogWrite(boostPin,boostPWM);
   
  if(boostPWM==0)
   switchToBuck=true;
       }
                  
    
  Serial.print("buck pwm: ");
  Serial.println(buckPWM);
  
  Serial.print("boost pwm: ");
  Serial.println(boostPWM);
}

void setup()
{
  Serial.begin(9600);
  
  //*********************switching setup********************************
  TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  analogWrite(buckPin,0);
  analogWrite(boostPin,0);
  vout=0;
  for(int i=0;i<numReadings;i++){
  vout = vout + analogRead(A1);
                      }
  vout=vout/numReadings;
  vout=vout*0.0493;
  Serial.print(vout);
  Serial.println(" V");

  buckPID.SetMode(AUTOMATIC);
  boostPID.SetMode(AUTOMATIC);
  
  //*********************end switching setup********************************
  
  myGLCD.InitLCD();

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  
  mainScreenDraw();
  

}

void loop()
{
  if(state==1)
    mainScreenDraw();
    
    
  else if(state==2)
  {
    regulate();
  if (myTouch.dataAvailable())
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      
      if ((x>=220) && (x<=360) && (y>=120) && (y<=184))  // Button: Set VOUT
        {
          Serial.println("key press");
          state=3;
        }
    }
   
   
  //VOUT real
  if(voutRealDisplayCount++==60)
  {
    myGLCD.setColor(0,255,0);//green
    myGLCD.printNumF(vout,1,220,40);//1 decimal place, x=40, y=40, 46 = ".", 3 characters, 48= "0" (filler)
    voutRealDisplayCount=0;
  }
  
  
  //VIN
  if(vinDisplayCount++==100)
  {
  
  vin=0;
  for(int i=0;i<5;i++)
  vin=vin+analogRead(A7);
  vin=vin/5;
  vin=vin*0.0196;
  vinDisplayCount=0;
  
  myGLCD.setColor(0, 0, 255);//blue
  myGLCD.printNumF(vin,1,20,120);//1 decimal place, x=40, y=40, 46 = ".", 3 characters, 48= "0" (filler)
  }
  
  
  }//end of state 2
    
  
  else if (state==3)
  {
    inputScreenDraw();
  }//end of state 3

  
  else if (state==4)//input screen loop
  {
     regulate();
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();

      if ((y>=10) && (y<=60))  // Upper row
      {
        if ((x>=10) && (x<=60))  // Button: 1
        {
          waitForIt(10, 10, 60, 60);
          updateStr('1');
        }
        if ((x>=70) && (x<=120))  // Button: 2
        {
          waitForIt(70, 10, 120, 60);
          updateStr('2');
        }
        if ((x>=130) && (x<=180))  // Button: 3
        {
          waitForIt(130, 10, 180, 60);
          updateStr('3');
        }
        if ((x>=190) && (x<=240))  // Button: 4
        {
          waitForIt(190, 10, 240, 60);
          updateStr('4');
        }
        if ((x>=250) && (x<=300))  // Button: 5
        {
          waitForIt(250, 10, 300, 60);
          updateStr('5');
        }
      }

      if ((y>=70) && (y<=120))  // Center row
      {
        if ((x>=10) && (x<=60))  // Button: 6
        {
          waitForIt(10, 70, 60, 120);
          updateStr('6');
        }
        if ((x>=70) && (x<=120))  // Button: 7
        {
          waitForIt(70, 70, 120, 120);
          updateStr('7');
        }
        if ((x>=130) && (x<=180))  // Button: 8
        {
          waitForIt(130, 70, 180, 120);
          updateStr('8');
        }
        if ((x>=190) && (x<=240))  // Button: 9
        {
          waitForIt(190, 70, 240, 120);
          updateStr('9');
        }
        if ((x>=250) && (x<=300))  // Button: 0
        {
          waitForIt(250, 70, 300, 120);
          updateStr('0');
        }
        if ((x>=310) && (x<=360))  // Button: "."
        {
          waitForIt(310, 70, 360, 120);
          updateStr('.');
        }
      }

      if ((y>=130) && (y<=180))  // Lower row
      {
        if ((x>=10) && (x<=150))  // Button: Clear
        {
          waitForIt(10, 130, 150, 180);
          myGLCD.print("                    ", CENTER, 200);
          stCurrent[0]='\0';
          stCurrentLen=0;

        }
        if ((x>=160) && (x<=300))  // Button: Enter
        {
          waitForIt(160, 130, 300, 180);
          if (stCurrentLen>0)
          {
            double v=StrToFloat(stCurrent);
            Serial.println(v);
            stCurrent[0]='\0';
            stCurrentLen=0;
            if(v>32)
            {
              myGLCD.print("                     ", LEFT, 192);
              myGLCD.print("                    ", CENTER, 200);
              myGLCD.setColor(255, 0, 0);
              myGLCD.print("Range is 0->32 V", LEFT, 192);
              delay(500);
              myGLCD.print("                     ", LEFT, 192);
              delay(500);
              myGLCD.print("Range is 0->32 V", LEFT, 192);
              delay(500);
              myGLCD.setColor(0, 255, 0); 
              myGLCD.print("                     ", LEFT, 192);
              myGLCD.print("                    ", CENTER, 200);
            }
            else
              {
              state=1;
              vref=v;
              }
            
          }
          else
          {
            myGLCD.setColor(255, 0, 0);
            myGLCD.print("Please Enter a value", LEFT, 192);
            delay(500);
            myGLCD.print("                     ", LEFT, 192);
            delay(500);
            myGLCD.print("Please Enter a value", LEFT, 192);
            delay(500);
            myGLCD.print("                     ", LEFT, 192);
            myGLCD.setColor(0, 255, 0);
          }
        }
      }
    }
  }//end  if (setVoltage==true)
}

float StrToFloat(String str){
  char carray[str.length() + 1]; //determine size of the array
  str.toCharArray(carray, sizeof(carray)); //put str into an array
  return atof(carray);
}

