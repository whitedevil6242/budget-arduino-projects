#include <Servo.h>

Servo myServo;

const int servoPin = 9;
const int buttonPin = 2;
const int potPin = A0;
const int buzzerPin = 3;   // Buzzer on pin 3

int buttonState;
int lastButtonState = HIGH;
bool servoAtZero = true;  // Start at 0°

void setup() {
  myServo.attach(servoPin);
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(buzzerPin, OUTPUT);
  myServo.write(0);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    if (servoAtZero) {
      beep();               // Beep when moving forward
      moveServo(0, 180);
      servoAtZero = false;
    } else {
      beep();               // Beep when moving back
      moveServo(180, 0);
      servoAtZero = true;
    }
    delay(200); // debounce
  }

  lastButtonState = buttonState;
}

// Servo movement with potentiometer speed (slower range)
void moveServo(int startAngle, int endAngle) {
  int potValue = analogRead(potPin);                // 0–1023
  int stepDelay = map(potValue, 0, 1023, 30, 5);  // Slowest=120ms, Fastest=20ms

  if (startAngle < endAngle) {
    for (int pos = startAngle; pos <= endAngle; pos++) {
      myServo.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = startAngle; pos >= endAngle; pos--) {
      myServo.write(pos);
      delay(stepDelay);
    }
  }
}

// Simple beep function
void beep() {
  tone(buzzerPin, 1000, 200); // 1000Hz, 200ms
  delay(200);                 // wait for beep to finish
}
