
/*
 * 
haydent - citroen c5 with am6 6 speed gearbox
emulate lexia actia diagnostics tool to request stats
log stats and general gearbox state to sd card
https://hackaday.io/project/12644-can-diagnostics-logger-with-arduino-can-shield

Logs:

X-Axis:
  Millis : Milliseconds since script start
Y1-Axis:
  Gearbox gear engaged : -2 to 6
  Sequence electrovalve S1 & S2 : The electrovalve is activated in gear 1 with engine braking
Y2-Axis:
  Converter lock-up pressure modulating electrovalve
  Line pressure modulating electrovalve
  Brake B1 : The electrovalve is activated in R,P,N and in gears 1,3,4,5
  Clutch C1 : The electrovalve is activated in R,P,N and in gears 5 and 6
  Clutch C2 : The electrovalve is activated in R,P,N and in gears 1,2 and 3
  Clutch C3 : The electrovalve is activated in R,P,N and in gears 1,2,4 and 6

Control current is state requested by ECU : 100mA not activated, 1000mA activated
Measured current is actual state of valve : <=200mA not activated, >200mA <900mA controlled, 900mA=> <1333mA activated

*/

#define DEBUG 0
#define PRINT_HEX 0
#define DIAG_DELAY 250//milli delay between diag requests. valve block data takes 45 milli to return.


#include <SPI.h>
#include <SD.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 10;//slave/cable select for can bus shield
MCP_CAN CAN(SPI_CS_PIN);// Set CS pin

const int diagTXID = 0x6A9;//diagnostics can request id
const int diagRXID = 0x689;//diagnostics can reply id
const int gearRXID = 0x489;//gearbox can data id

int connectionStage = 0;
unsigned long diagTimer = 0;
int valveState[16];
int currentGear;
int lastGear;

#include "card.h"
#include "display.h"

void setup()
{
    Serial.begin(9600);
    
    initCard();
    initDisplay();

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
      
  CAN.init_Mask(0,0,0x03FF);
  CAN.init_Mask(1,0,0x03FF);
  CAN.init_Filt(0,0,gearRXID);
  CAN.init_Filt(1,0,diagRXID);

}



void loop()
{
    unsigned char len = 0;
    unsigned char rxBuffer[8];

    updateDisplay();    
    
    if(!connectionStage)connectAM6(1);//start diag connection
    if(connectionStage==4 && (millis() - diagTimer > DIAG_DELAY))requestValveData(1);//request valve data
    

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, rxBuffer);    // read data,  len: data length, buf: data buf

        int canId = CAN.getCanId();

            if(canId == gearRXID){//general gearbox can data
              processGears(rxBuffer);
            }
            else if(canId == diagRXID){ //diagnostic parameter response 
                diagTimer = millis();
                
              if(connectionStage != 4){//still connecting
                 
                  switch (rxBuffer[0])
                  {
                    case 0x03://step 1 response
                         if(rxBuffer[1]==0xC1 && rxBuffer[2]==0xD0 && rxBuffer[3]==0x8F)connectAM6(2);
                      break;
                    case 0x10://step 2 response//continue//maybe just 0x10
                         if(rxBuffer[1]==0x18 && rxBuffer[2]==0x61 && rxBuffer[3]==0x80 && rxBuffer[4]==0x96 && rxBuffer[5]==0x54 && rxBuffer[6]==0x49 && rxBuffer[7]==0x34)connectAM6(3);
                      break;
                    case 0x23://step 3 response, we are now connected
                         if(rxBuffer[1]==0xFF && rxBuffer[2]==0xFF && rxBuffer[3]==0xFF && rxBuffer[4]==0xFF)
                         {
                           connectionStage=4;
                           requestValveData(1);//request valve data
                           Serial.println("diag connected");                           
                         }
                      break;
                    default:
                          Serial.println("unknown message");
                      break;
                  }

              }
              else//is connected
              {
                  int arrayPos = 0;

                  switch (rxBuffer[0])
                  {
                    case 0x01:
                         if(rxBuffer[1]==0x7E){                          
                          if(DEBUG)Serial.println("ping response");//ping response
                         }
                      break;
                    case 0x10://continue//first line of valve data 5of8 bytes
                         if(DEBUG)Serial.println("first line of valve data");
                         requestValveData(2);
                      break;
                    case 0x21://second line of valve data 7of8 bytes
                          if(DEBUG)Serial.println("second line of valve data");
                          arrayPos = 7;
                      break;
                    case 0x22://third final line of valve data 2of3 bytes
                         if(DEBUG)Serial.println("third line of valve data");
                         arrayPos = 14;
                         //reset for next request
                      break;
                    default:
                          Serial.println("unknown message");    
                      break;
                  }

              
                  for(int i = 1; i<len; i++)    // transfer the data
                  {
                       valveState[arrayPos+i-1] = rxBuffer[i];//hex byte
                  }

                  
                  if(arrayPos == 14)//recieved all valve data, process
                  {    
                   
                      //rewrite first 4 bytes with 4 bits of 4th byte
                     //Sequence S1 & S2 valve state, only on/off
                        valveState[0] = (valveState[3]&B10000000)>>7;
                        valveState[1] = (valveState[3]&B01000000)>>6;
                        valveState[2] = (valveState[3]&B00100000)>>5;
                        valveState[3] = (valveState[3]&B00010000)>>4;
                    
                        Serial.print("Data: "+String(currentGear)+" ");
                        String dataString = String(millis())+","+String(currentGear);
                        for(int i = 0; i<16; i++)    // print the data
                        {
                            if(i > 3)//valveState 5 to 16, milliamp
                            {
                              valveState[i] *= 10;//* 10 = valve milliamp current
                              if(valveState[i] >= 900)digitalWrite(ledArray[i],HIGH);
                              else digitalWrite(ledArray[i],LOW);
                            }
                            else//valveState 1-4, on/off
                            {
                              if(valveState[i])digitalWrite(ledArray[i],HIGH);
                              else digitalWrite(ledArray[i],LOW);
                            }
                          
                            if(PRINT_HEX)Serial.print(valveState[i], HEX);
                            else
                            {
                              char padded[3];
                              sprintf(padded, "%3d", valveState[i]);
                              Serial.print(padded);
                            }
                            Serial.print(" ");

                            dataString += ","+String(valveState[i]);
                        }
                        Serial.println();
                        
                        logData(dataString);
                  }
                  
                }
            }
    }
    
    if(millis() - diagTimer > 5000)connectAM6(0);//assume lost diag connection
}


void processGears(unsigned char rxBuffer[8]){  
    
      int gear = 0;
  
            switch (rxBuffer[0])
            {
                  case 0x00://P
                      
                    break;
                  case 0x91://R
                      
                    break;
                  case 0xA2://N
                       
                    break;
                  case 0x13://D1 (Drive 1)
                  case 0x17://S1 (Sequential 1)
                       gear = 1;
                    break;
                  case 0x23://D2
                  case 0x26://S2
                        gear = 2;
                    break;
                  case 0x33://D3
                  case 0x35://S3
                        gear = 3;
                    break;
                  case 0x43://D4
                  case 0x44://S4
                        gear = 4;
                    break;   
                  case 0x53://D5
                  case 0x5E://S5
                        gear = 5;
                    break;       
                  case 0x63://D6
                  case 0x6C://S6
                        gear = 6;
                    break;
                  default:
                      gear = currentGear;
                      Serial.println("unknown gear code");
                    break;
               }

              switch (rxBuffer[1])
              {
                case 0x04://sport

                  break;
                case 0x0C://snow
                 
                  break;
                case 0x08://sequential
                 
                  break;
              }
              
              switch (rxBuffer[2])
              {
                case 0x40://shift lock relay off/closed

                  break;
                case 0x00://shift lock relay on/open

                  break;
              }
              

              if(currentGear != gear)//gear change
              {
                lastGear = currentGear;
                currentGear = gear;
                if(DEBUG)Serial.println("Gear: "+String(currentGear));
              }
              
              
              //unknown what last 5 bytes mean
              
}


void connectAM6(int stage){//init diag session with am6 gearbox

    connectionStage = stage;
    
    switch (stage) {
      case 0://close connection
      {
          diagTimer=millis();
          unsigned char txBuffer[] = {0x01, 0x82};
          CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          Serial.println("diag connection, closed/lost");
      }
        break;       
      case 1://step 1
        {
          unsigned char txBuffer[] = {0x01, 0x81};
          CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          Serial.println("diag connection, starting");
        }
         break;
      case 2://step 2
      {
          unsigned char txBuffer[] = {0x02, 0x21, 0x80};
          CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          if(DEBUG)Serial.println("diag connection, stage 2");
      }
        break;
      case 3://step 3
      {
          unsigned char txBuffer[] = {0x30, 0x00, 0x14};//ready for next chunk of data
          CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          if(DEBUG)Serial.println("diag connection, stage 3");
      }
        break;
      case 4://keep alive/ping
      {
          unsigned char txBuffer[] = {0x01, 0x3E};
          CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          if(DEBUG)Serial.println("diag connection, ping");
      }
        break;
   
  }
      
}

void requestValveData(int stage){
  //lexia paramater data requested in blocks 
  //shift lock in C6, sport in C0, electro valves in C3
              
       switch (stage) {
          case 1://step 1
          {
              unsigned char txBuffer[] = {0x04, 0x21, 0xC3, 0x80, 0x01};//block C3, valve data
              if(DEBUG)Serial.println("valve step 1");
              CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
              diagTimer = millis();              
          }
            break;
          case 2://step 2
          {
              unsigned char txBuffer[] = {0x30, 0x00, 0x14};//ready for next chunk of data
              if(DEBUG)Serial.println("valve step 2");
              CAN.sendMsgBuf(diagTXID, 0, sizeof(txBuffer), txBuffer);
          }
            break;
       }
       
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
