#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define volumePotPin A0 // analog pin used for potentiometer to adjust volume
#define buttonPin 3

byte buttonPushCounter = 1;
long lastDebounceTime = 0;
int debounceDelay = 150;
volatile int buttonState = 0;
byte lastTrack = 1;
byte volume = 0;
byte lastVolume = 0;
byte volumeBeforeLast = 0;
int analogVolumeValue = 0;
int lastAnalogVolumeValue = -1;

SoftwareSerial mySerial(10, 11); // pins used for RX, TX
DFRobotDFPlayerMini myPlayer;

void setup() {
  // put your setup code here, to run once:
  mySerial.begin(9600);
  Serial.begin(9600);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myPlayer.begin(mySerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myPlayer.volume(volume);  //Set volume value. From 0 to 30

  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), pin_ISR, CHANGE);

  myPlayer.enableLoopAll();
  myPlayer.loop(buttonPushCounter);  //Play the first mp3
}

void pin_ISR() {

  buttonState = digitalRead(buttonPin);
  
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == 1) {
      Serial.println("Button Pressed");
      
      if (buttonPushCounter >= 6) {
        buttonPushCounter = 1;
      } else {
        buttonPushCounter++;
      }
      Serial.println(buttonPushCounter);
    }
  
  }
  lastDebounceTime = millis();

}

void adjustVolume(){
//  int analogAvg = 0;
//  for(int i = 0; i <= 10; i++){
//    analogAvg = analogAvg + analogRead(volumePotPin);
//  }
//  analogVolumeValue = analogAvg / 10;
  analogVolumeValue = analogRead(volumePotPin);

  volume = map(analogVolumeValue, 0, 3968, 0, 30);
  if (volume == volumeBeforeLast){
    return;
  }else if (volume != lastVolume){
      myPlayer.volume(volume);
      
      volumeBeforeLast = lastVolume;
      lastVolume = volume;
      
      Serial.println("Volume changed");
      Serial.print("Volume: ");
      Serial.println(volume);
  }
  
}

void setTrack(){
  myPlayer.loop(buttonPushCounter);
  Serial.println("Track changed");
  lastTrack = buttonPushCounter;
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (myPlayer.available()) {
    printDetail(myPlayer.readType(), myPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  if (buttonPushCounter != lastTrack){
    Serial.println("Trying to change track");
    setTrack();
  }
  adjustVolume();
  
  
}