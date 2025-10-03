#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin setup
const int ledPins[5] = {2, 3, 4, 5, 6};     // LEDs
const int btnPins[5] = {7, 8, 9, 10, 11};   // Buttons
const int activeBuzzer = 12;                // Hit/Error sound
const int passiveBuzzer = 13;               // BGM

int score = 0;
unsigned long gameTime = 30000; // 30s game duration
unsigned long startTime;
bool gameRunning = true;

// --------- BGM setup ----------
int melody[] = {262, 294, 330, 349, 392, 440}; // C D E F G A
int noteDurations[] = {300, 300, 300, 300, 300, 300}; 
int currentNote = 0;
unsigned long lastNoteTime = 0;
// -------------------------------

void setup() {
  for (int i = 0; i < 5; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH); // all OFF at start (inverted)
    pinMode(btnPins[i], INPUT_PULLUP); // active LOW
  }
  
  pinMode(activeBuzzer, OUTPUT);
  pinMode(passiveBuzzer, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // stop if OLED not found
  }

  splashScreen();
  startTime = millis();
}

void loop() {
  if (gameRunning) {
    playBGM();     // background melody
    playGame();    // game loop
  } else {
    checkRestart();
  }
}

void playGame() {
  unsigned long currentTime = millis();
  if (currentTime - startTime >= gameTime) {
    gameOver();
    return;
  }

  int mole = random(0, 5);
  digitalWrite(ledPins[mole], LOW);  // turn ON mole (inverted logic)

  unsigned long appearTime = millis();
  bool moleHit = false;

  while (millis() - appearTime < 1000) {
    // Check for correct hit
    if (digitalRead(btnPins[mole]) == LOW) {
      score++;
      tone(activeBuzzer, 1000, 100); // hit sound (high beep)
      moleHit = true;
      break;
    }

    // Check for wrong button press
    for (int i = 0; i < 5; i++) {
      if (i != mole && digitalRead(btnPins[i]) == LOW) {
        score--;
        if (score < 0) score = 0; // no negative score
        tone(activeBuzzer, 200, 200); // error sound (low tone)
        break;
      }
    }

    if (millis() - startTime >= gameTime) {
      gameOver();
      return;
    }

    playBGM(); // keep BGM alive during wait
  }

  digitalWrite(ledPins[mole], HIGH); // OFF
  delay(200);
  updateDisplay();
}

// ----------- BGM Function -------------
void playBGM() {
  unsigned long now = millis();
  if (now - lastNoteTime > noteDurations[currentNote]) {
    tone(passiveBuzzer, melody[currentNote], noteDurations[currentNote] - 50);
    currentNote++;
    if (currentNote >= 6) currentNote = 0; // loop melody
    lastNoteTime = now;
  }
}
// --------------------------------------

void updateDisplay() {
  unsigned long currentTime = millis();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Score:");
  display.setCursor(70, 0);
  display.println(score);

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Time: ");
  display.print((gameTime - (currentTime - startTime)) / 1000);
  display.print("s");
  display.display();
}

void gameOver() {
  gameRunning = false;

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.println("GAME OVER");
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("Final Score: ");
  display.println(score);
  display.display();
}

void checkRestart() {
  bool allPressed = true;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(btnPins[i]) == HIGH) {
      allPressed = false;
      break;
    }
  }

  if (allPressed) {
    score = 0;
    startTime = millis();
    gameRunning = true;
    splashScreen();
  }
}

void splashScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Whack-a-Mole!");
  display.setCursor(0, 16);
  display.println("Press buttons to play");
  display.display();
  delay(2000);
}
