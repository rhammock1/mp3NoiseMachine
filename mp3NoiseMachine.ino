#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "FastLED.h"
#include <avr/sleep.h>

#define wakeUpPin 2 // pin used to wake Arduino from sleep
#define volumePotPin A0 // analog pin used for potentiometer to adjust volume
#define buttonPin 3
#define LED_PIN 13
#define NUM_LEDS 10

long timeToSleep = 14400000; // (4 hours in ms)

long counter = 0;
bool down = false;
int brightness = 0;

byte buttonPushCounter = 6;
long lastDebounceTime = 0;
int debounceDelay = 150;
volatile int buttonState = 0;

byte lastTrack = 6;
byte volume = 0;
byte lastVolume = 0;
byte volumeBeforeLast = 0;
int analogVolumeValue = 0;
int lastAnalogVolumeValue = -1;

SoftwareSerial mySerial(10, 11); // pins used for RX, TX
DFRobotDFPlayerMini myPlayer;
CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  mySerial.begin(9600);
  Serial.begin(115200);
  
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
  
  FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.clear();
  
  myPlayer.enableLoopAll();
  myPlayer.play(buttonPushCounter);  //Play the last mp3 - guided sleep meditation on my SD card
}

void goToSleep(){
  
  Serial.println("Getting ready to go to sleep now boss");
  sleep_enable(); // enable sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  FastLED.clear(true);
  myPlayer.pause();
  delay(1000); // wait a second to allow leds to clear and mp3 to pause
  sleep_cpu(); // activate sleep mode
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

int handleBrightness() {
// responsible for the "breathing" effect

  brightness = sin(counter / 800.0 * PI) * 1000.0; // calculates sin wave 
  brightness = map(brightness, -1000, 1000, 0, 75); // maps that value to be between 0 and 100

  if (!down) {
    counter = counter + 5; // determines amplitude of the wave
  } else {
    counter = counter - 5;
  }
  if (brightness >= 75) {
    down = true;
    brightness = 75;
  } else if (brightness <= 0) {
    down = false;
    brightness = 0;
  }
  
  return brightness;
}

void handleLED(){
  
  fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
  FastLED.setBrightness(handleBrightness());
  FastLED.show();
  
  delay(25);
}

void adjustVolume(){

  analogVolumeValue = analogRead(volumePotPin);

  volume = map(analogVolumeValue, 0, 3968, 0, 25);
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
      if (buttonPushCounter == 6) {
        Serial.println("Last file finished, resetting buttonPushCounter to 2"); // currently file 2 is preferred over others
        buttonPushCounter = 2;
        setTrack();
      }
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
  if (millis() >= timeToSleep){
    goToSleep();
  }
  if (myPlayer.available()) {
    printDetail(myPlayer.readType(), myPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
  
  if (buttonPushCounter != lastTrack){
    Serial.println("Trying to change track");
    setTrack();
  }

  adjustVolume();
  handleLED();
  
}