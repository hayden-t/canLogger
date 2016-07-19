// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 49;
int LED_PIN = 41;

File dataFile;

void initCard(void)
{

  pinMode(41, OUTPUT);
  
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  File dir = SD.open("/");
  int fileCount=0;
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry)break;// no more files 

    fileCount++;
    entry.close();
  }

  String logName = "log"+String(fileCount)+".csv";
  dataFile = SD.open(logName, FILE_WRITE);

  //column headings
  dataFile.println("Milli,Gear,S1_Control,S1_Return,S2_Control,S2_Return,Converter_Control,Converter_Measured,Line_Control,Line_Measured,B1_Control,B1_Measured,C1_Control,C1_Measured,C2_Control,C2_Measured,C3_Control,C3_Measured");
  dataFile.flush();
  
  Serial.println(logName + " started");
  
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

void logData(String dataString){

  static boolean ledState = false;

  if(!ledState){digitalWrite(LED_PIN, HIGH);ledState=true;}
  else{ digitalWrite(LED_PIN, LOW);ledState=false;}
  
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.flush();
    // print to the serial port too:
    if(DEBUG)Serial.println("Logged: "+dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening log");
  }
  
}

