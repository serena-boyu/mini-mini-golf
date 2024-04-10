#include <Arduino.h>          // for restart function
#include <avr/wdt.h>          // Include AVR watchdog library
#include "FastLED.h"          // for LED strip
#include <TimerOne.h>         // for interupt timing
#include <DFRobot_DF1201S.h>  // for speakers
#include <SoftwareSerial.h>   // for speakers
#include <Servo.h>            // include the servo library

void (*resetFunc)(void) = 0;  // Declare reset function at address 0

// medusa servo
#define MEDUSA_SERVO 9  // pin # servo is plugged into
Servo myservo;
int medusaHiding = 10;   // servo position
int medusaAttack = 170;  // servo position

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

// for implementing interupt timing
const int interval1 = 7000;

// for implementing state machine timing
unsigned long timeToChangeMedusa = 0;
int state = 0;
#define MEDUSA_STATE_CALM 0
#define MEDUSA_STATE_ANGRY 1


// -----------

// set up medusa state as a boolean
bool medusaAngry = true;
int tDelay = 1000;     // 1000 = 1 seconds
int evilDelay = 5000;  // 5000 = 5 seconds
bool win = false;
int speakerVolume = 20;  // set a low-ish volume (valid values from 0 to 30)


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("init");

  // set up break beam sensor
  pinMode(LEDPIN, OUTPUT);        // initalize the LED pin as an output
  pinMode(SENSORPIN, INPUT);      // initialize the sensor pin as an input
  digitalWrite(SENSORPIN, HIGH);  // turn on the pullup

  // set up LED strip
  FastLED.addLeds<LED_TYPE, LED_STRIP_PIN>(leds, LED_COUNT);
  FastLED.show();  // Initialize all pixels to 'off'

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

  // set up medusa servo
  // starting position for medusa servo
  Serial.println("init servo");
  pinMode(MEDUSA_SERVO, OUTPUT);
  myservo.attach(MEDUSA_SERVO);  // tell the servo object that its servo is plugged into SNAKE_SERVO (pin 9)
  myservo.write(medusaHiding);
  Serial.println("starting");
}

// ----------------------------------------------

// FUNCTIONS:

void restartArduino() {
  wdt_enable(WDTO_15MS);  // Enable watchdog timer with 15ms timeout
  while (1) {}            // Wait for watchdog timer to reset the Arduino
}

void isMedusaAngry() {
  return medusaAngry;
}

void editMedusaAngry(bool b) {
  medusaAngry = b;
}

void medusaHide() {
  myservo.write(medusaHiding);
}

void medusaShow() {
  myservo.write(medusaAttack);
}

// LED FUNCTIONS:
void turnLEDStripWhite() {
  // Turn off all LEDs
  delay(evilDelay);
  fill_solid(leds, LED_COUNT, CRGB::White);
  FastLED.show();  // Update the LED strip
}

void turnLEDStripRed() {
  // Turn off all LEDs
  delay(250);
  fill_solid(leds, LED_COUNT, CRGB(0, 255, 0));  // red
  FastLED.show();                                // Update the LED strip
}

void turnLEDStripOff() {
  // Turn off all LEDs
  delay(tDelay);
  fill_solid(leds, LED_COUNT, CRGB::Black);  // off
  FastLED.show();                            // Update the LED strip
}

void turnLEDStripYellow() {
  delay(250);
  fill_solid(leds, LED_COUNT, CRGB::Yellow);
  FastLED.show();
}

// SPEAKER FUNCTIONS:
void medusaSound() {
  digitalWrite(13, LOW);
  int result = DF1201S.playFileNum(1);  // turn on LED if it play says it worked
  Serial.println("MedusaSound");
  if (result == 1) {                    // turn on Arduino LED if board says it worked
    digitalWrite(13, HIGH);
  }
  delay(evilDelay);
  DF1201S.pause();  // Pause playback after 5 seconds
}

void yaySound() {
  digitalWrite(13, LOW);
  int result = DF1201S.playFileNum(2);  // turn on LED if it play says it worked
  if (result == 1) {                    // turn on Arduino LED if board says it worked
    digitalWrite(13, HIGH);
  }
  delay(1000);
}

// what to do when someone wins
// set medusa angry state to false
// play happy music
// change color of house
void celebrate() {
  Serial.println("CELEBRATE");
  medusaAngry = false;
  turnLEDStripYellow();
  yaySound();
  turnLEDStripOff();
}

void checkIfWinner() {
  Serial.println("checking for a winner...");
  // celebrate if win is true
  if (!sensorState && lastState && (state == MEDUSA_STATE_CALM)) {
    medusaAngry = false;
    Serial.println("Winner");
    win = true;
    celebrate();
    win = false;
    delay(1000);
    medusaAngry = true;
    restartArduino();
  }
  lastState = sensorState;
}

// -------------

// DIFFERENT STATES

// runs every X seconds
void stateMachine() {
  Serial.println(state);
  unsigned long now = millis();  // elapsed time
  if (state == MEDUSA_STATE_CALM) {
    Serial.println("CALM ANGRY");
    state = MEDUSA_STATE_ANGRY;
    turnLEDStripOff();
    medusaHide();
    timeToChangeMedusa = now + 5000;
  }

  else if (state == MEDUSA_STATE_ANGRY) {
    state = MEDUSA_STATE_CALM;
    Serial.println("CALM STATE");
    medusaShow();
    turnLEDStripRed();
    medusaSound();
    timeToChangeMedusa = now + 8000;
  }
}

// ----------------------------------------------

void loop() {
  
  if (millis() >= timeToChangeMedusa) {
    Serial.println('state change');
    stateMachine();
  }
  

  // sense if breakbeam is triggered, which means someone has completed the course
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);
  // check if the sensor beam is broken
  // if it is, the sensorState is LOW:
  if (sensorState == LOW) {
    // turn LED on:
    digitalWrite(LEDPIN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LEDPIN, LOW);
  }

  if (sensorState && !lastState) {
    Serial.println("No Winner");
    win = false;
  }

  // celebrate if win is true
  checkIfWinner();
  
}
