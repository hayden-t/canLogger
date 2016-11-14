// LCD5110_NumberFonts (C)2013 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// This program is a demo of the included number-fonts,
// and how to use them.
//
// This program requires a Nokia 5110 LCD module.
//
// It is assumed that the LCD module is connected to
// the following pins using a levelshifter to get the
// correct voltage to the module.
//      SCK  - Pin 8
//      MOSI - Pin 9
//      DC   - Pin 10
//      RST  - Pin 11
//      CS   - Pin 12
//

//#include <LCD5110_Basic.h>
#include <LCD5110_Graph.h>

LCD5110 myGLCD(3,4,5,7,6);//light 2

#define BACKLIGHT A4
//
//#define LED1 A0
//#define LED2 A1
//#define LED3 A3
//#define LED4 A2
//#define LED5 9
//#define LED6 8
//#define LED7 11
//#define LED8 12

#define BAR_WIDTH 6//pixels

//odd = control, even = measured
//int ledArray[] = {-1,LED1,-1,LED2,-1,LED3,-1,LED4,-1,LED5,-1,LED6,-1,LED7,-1,LED8};

extern uint8_t TinyFont[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

void initDisplay()
{
  //led's
   pinMode(BACKLIGHT, OUTPUT);
   
//   pinMode(LED1, OUTPUT);
//   pinMode(LED2, OUTPUT);
//   pinMode(LED3, OUTPUT);
//   pinMode(LED4, OUTPUT);
//   pinMode(LED5, OUTPUT);
//   pinMode(LED6, OUTPUT);
//   pinMode(LED7, OUTPUT);
//   pinMode(LED8, OUTPUT);

  //lcd
    myGLCD.InitLCD();
    
   digitalWrite(BACKLIGHT,LOW);//backlight on


   
}

void drawBar(int x, int height){

  height = constrain(height, 0, 100);
  height = map(height, 0, 100, 47, 6);

  x = constrain(x, 0, 84);
   
   for(int i = 0; i <= BAR_WIDTH; i++){
      myGLCD.clrLine(x+i, 6, x+i, 48);
      myGLCD.drawLine(x+i,height,x+i,48);
     
   }
   myGLCD.update();

}

void drawGraphs(void){
  
  int valveMap[] = {5, 7, 9, 11, 13, 15};

  for(int i = 0;i < 6; i++){
      int percent = map(valveState[valveMap[i]], 0, 1333, 0, 100);
      drawBar(((i*2)*BAR_WIDTH)+3, percent);
  }


}

void updateDisplay(void)
{

  
   myGLCD.setFont(TinyFont);
   myGLCD.print("TC LP B1 C1 C2 C3", 3, 0);
   myGLCD.print("ON", RIGHT, 22);
   myGLCD.print("OF", RIGHT, 35);

   int off = map(200, 0, 1333, 47, 6);
   int on = map(900, 0, 1333, 47, 6);   
   
   myGLCD.drawLine(0,off,84,off);
   myGLCD.drawLine(0,on,84,on);

   myGLCD.setFont(MediumNumbers);
   myGLCD.printNumI(currentGear, RIGHT, 0);

   myGLCD.update();

}




