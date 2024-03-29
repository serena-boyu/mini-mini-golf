#include <Servo.h>  // include the servo library

int servoPin = 9;          // pin # servo is plugged into
int servoPosition1 = 0;    //servo will move into this position'
int servoPosition2 = 180;  //servo will move into this position

bool eventTrigger = false;  // when to trigger servo movement

#define LEDPIN 13

#define SENSORPIN 4
int sensorState = 0, lastState = 0;  // variable for reading the pushbutton status

Servo myservo;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(servoPin);  // tell the servo object that its servo is plugged into servoPin (pin 9)

  pinMode(LEDPIN, OUTPUT);        // initalize the LED pin as an output
  pinMode(SENSORPIN, INPUT);      // initialize the sensor pin as an input
  digitalWrite(SENSORPIN, HIGH);  // turn on the pullup

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  myservo.write(servoPosition1);
  delay(1000);  // wait 1 second

  // ---------------------------------------------------

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
    Serial.println("Unbroken");
    eventTrigger = false;
  }
  if (!sensorState && lastState) {
    Serial.println("Broken");
    eventTrigger = true;

    // move servo if eventTrigger is true
    if (eventTrigger == true) {
      myservo.write(servoPosition2);
      delay(1000);  // wait 1 second
      myservo.write(servoPosition1);
      Serial.println("Event Triggered");
    }
  }
  lastState = sensorState;
}
