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

#include <LCD5110_Basic.h>

LCD5110 myGLCD(3,4,5,7,6);//light 2

#define BACKLIGHT A4

#define LED1 A0
#define LED2 A1
#define LED3 A3
#define LED4 A2
#define LED5 9
#define LED6 8
#define LED7 11
#define LED8 12

//odd = control, even = measured
int ledArray[] = {-1,LED1,-1,LED2,-1,LED3,-1,LED4,-1,LED5,-1,LED6,-1,LED7,-1,LED8};


extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

void initDisplay()
{
  //led's
   pinMode(BACKLIGHT, OUTPUT);
   
   pinMode(LED1, OUTPUT);
   pinMode(LED2, OUTPUT);
   pinMode(LED3, OUTPUT);
   pinMode(LED4, OUTPUT);
   pinMode(LED5, OUTPUT);
   pinMode(LED6, OUTPUT);
   pinMode(LED7, OUTPUT);
   pinMode(LED8, OUTPUT);

  //lcd
    myGLCD.InitLCD();
    myGLCD.InitLCD();
    
   digitalWrite(BACKLIGHT,LOW);//backlight on
   
}

void updateDisplay(void)
{

   myGLCD.setFont(BigNumbers);
   myGLCD.printNumI(currentGear, CENTER, 8);

   myGLCD.setFont(MediumNumbers);
   myGLCD.printNumI(lastGear, RIGHT, 0);
  
   myGLCD.setFont(MediumNumbers);
   myGLCD.printNumI((millis()/1000), RIGHT, 32);
}

