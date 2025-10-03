#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <DHT.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Moisture Sensor
#define sensor A2
#define redLED 3
#define greenLED 4
#define blueLED 5

// DFPlayer Mini
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// Moisture calibration
#define wet 511
#define dry 673

// DHT11
#define DHTPIN 2       // DHT11 data pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

unsigned long prevMillis = 0;
const long interval = 3000; // 3s change
bool wasThirsty = false;    // Track thirsty → watered state
bool showTemp = false;      // Toggle between plant msg and temp

void setup() {
  Serial.begin(9600);
  mySoftwareSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  dht.begin();

  if (!myDFPlayer.begin(mySoftwareSerial)) {
    lcd.setCursor(0, 0);
    lcd.print("DFPlayer Error!");
    while (true);
  }
  myDFPlayer.volume(25);
}

void loop() {
  int value = analogRead(sensor);
  int percent = map(value, wet, dry, 100, 0);
  percent = constrain(percent, 0, 100);

  if (millis() - prevMillis >= interval) {
    prevMillis = millis();
    lcd.clear();

    if (showTemp) {
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        lcd.setCursor(0, 0);
        lcd.print("DHT11 Error!");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(t);
        lcd.print((char)223); // degree symbol
        lcd.print("C");

        lcd.setCursor(0, 1);
        lcd.print("Humid: ");
        lcd.print(h);
        lcd.print("%");
      }
      showTemp = false;
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Moisture: ");
      lcd.print(percent);
      lcd.print("%   ");

      if (percent <= 30) {
        wasThirsty = true;
        String msgs[] = {"I'm DYING", "Help meeee!", "So thirsty!"};
        lcd.setCursor(0, 1);
        lcd.print(msgs[random(0, 3)]);
        myDFPlayer.play(random(0, 5)); // 0001–0004.mp3
        flashRed();
      } 
      else if (percent <= 80) {
        if (wasThirsty) {
          lcd.setCursor(0, 1);
          lcd.print("Yum, water!");
          myDFPlayer.play(6); // 0005.mp3
          wasThirsty = false;
        } else {
          String msgs[] = {"Yay", "Feeling fresh!", "So happy!"};
          lcd.setCursor(0, 1);
          lcd.print(msgs[random(0, 3)]);
        }
        greenLight();
      } 
      else {
        lcd.setCursor(0, 1);
        lcd.print("Zzz...");
        blueBreathing();
      }
      showTemp = true;
    }
  }
}

// LED patterns
void flashRed() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(redLED, HIGH);
    delay(200);
    digitalWrite(redLED, LOW);
    delay(200);
  }
}

void greenLight() {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(blueLED, LOW);
}

void blueBreathing() {
  for (int i = 0; i <= 255; i++) {
    analogWrite(blueLED, i);
    delay(5);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(blueLED, i);
    delay(5);
  }
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
}
