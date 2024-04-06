#include "FastLED.h"          // for LED strip
#include <TimerOne.h>         // for interupt timing
#include <DFRobot_DF1201S.h>  // for speakers
#include <SoftwareSerial.h>   // for speakers

// break beam sensor
#define LEDPIN 13
#define SENSORPIN 4
int sensorState = 0, lastState = 0;  // variable for reading the pushbutton status

// LED strip
#define LED_STRIP_PIN 8   // Arduino pin that connects to NeoPixel
#define LED_COUNT 36      // number of LEDs in strip (change as necessary)
#define LED_TYPE WS2812B  // type of LED strip (change if using a diff type)
CRGB leds[LED_COUNT];

// speakers
// Tell Arduino which pins are wired to MP3 player (remember RX on Arduino is TX on MP3 player)
SoftwareSerial DF1201SSerial(2, 3);  //Arduino RX, Arduino TX
DFRobot_DF1201S DF1201S;             // object that will handle all comms to MP3 player

// -----------

// set up medusa state as a boolean
bool medusaAngry = false;
int tDelay = 1000;     // 1000 = 1 seconds
int evilDelay = 5000;  // 5000 = 5 seconds
bool win = false;
int speakerVolume = 10;  // set a low-ish volume (valid values from 0 to 30)


void setup() {
  // put your setup code here, to run once:

  // set up LED strip
  FastLED.addLeds<LED_TYPE, LED_STRIP_PIN>(leds, LED_COUNT);
  FastLED.show();  // Initialize all pixels to 'off'

  // set up break beam sensor
  pinMode(LEDPIN, OUTPUT);        // initalize the LED pin as an output
  pinMode(SENSORPIN, INPUT);      // initialize the sensor pin as an input
  digitalWrite(SENSORPIN, HIGH);  // turn on the pullup

  Serial.begin(9600);

  // set up speakers
  pinMode(13, OUTPUT);
  // Initialize MP3 player communications
  DF1201SSerial.begin(115200);
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  DF1201S.setVol(speakerVolume);  // set a low-ish volume (valid values from 0 to 30)
  delay(500);
  DF1201S.switchFunction(DF1201S.MUSIC);  // be a music player, not a USB drive
  delay(500);
  DF1201S.setPlayMode(DF1201S.SINGLE);  // tell it to play just one file at a time (do this after switchFunction)
  delay(500);
}

// ----------------------------------------------

// FUNCTIONS:

void isMedusaAngry() {
  return medusaAngry;
}

void editMedusaAngry(bool b) {
  medusaAngry = b;
}

// LED FUNCTIONS:
void turnLEDStripWhite() {
  // Turn off all LEDs
  delay(evilDelay);
  fill_solid(leds, LED_COUNT, CRGB::White);
  FastLED.show();  // Update the LED strip
}

void turnLEDStripGreen() {
  // Turn off all LEDs
  delay(tDelay);
  fill_solid(leds, LED_COUNT, CRGB(255, 0, 0));  // green
  FastLED.show();                                // Update the LED strip
}

void turnLEDStripOff() {
  // Turn off all LEDs
  delay(tDelay);
  fill_solid(leds, LED_COUNT, CRGB::Black);  // off
  FastLED.show();                            // Update the LED strip
}

// SPEAKER FUNCTIONS:
void medusaSound() {
  digitalWrite(13, LOW);
  int result = DF1201S.playFileNum(2);  // turn on LED if it play says it worked
  if (result == 1) {                    // turn on Arduino LED if board says it worked
    digitalWrite(13, HIGH);
  }
  delay(4000);
}

// ----------------------------------------------

void loop() {
  /*
editMedusaAngry(false);
delay(1000);
*/

  turnLEDStripWhite();
  turnLEDStripGreen();
  turnLEDStripOff();

  medusaSound();

  // sense if breakbeam is triggered, which means someone has completed the course
  // set medusa angry state to false
  // play happy music
  // change color of house

  // if medusa state is angry... (on delay)
  // LED lights up
  // spooky music plays
  // snake is in attack mode
}