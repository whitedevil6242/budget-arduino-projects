#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define WHITE_LED 2
#define YELLOW_LED 3
#define GREEN_LED 4
#define RED_LED 5
#define BUZZER 6
#define START_BTN 7   // same button: start & react
#define RESET_BTN 8

bool gameRunning = false;

// Optional idle BGM (non-blocking)
int melody[] = {262, 294, 330, 294, 262, 0, 330, 349, 392, 0};
int noteDurations[] = {300, 300, 300, 300, 400, 200, 300, 300, 400, 200};
int melodyLength = sizeof(melody) / sizeof(melody[0]);
int currentNote = 0;
unsigned long lastNoteTime = 0;
bool playingNote = false;

void setup() {
  pinMode(WHITE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);

  digitalWrite(WHITE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.println("Reaction Test Ready");
  display.display();

  randomSeed(analogRead(A0));
}

void loop() {
  // idle BGM while waiting
  if (!gameRunning) playIdleBGMNonBlocking();

  // Start when button pressed (initial press)
  if (digitalRead(START_BTN) == LOW && !gameRunning) {
    // Debounce initial press
    delay(30);
    if (digitalRead(START_BTN) == LOW) {
      gameRunning = true;
      noTone(BUZZER);         // stop idle BGM immediately
      // Wait for the user to release the button before sequence begins
      while (digitalRead(START_BTN) == LOW) { delay(5); }
      delay(50); // small settle
      runReactionGame();
    }
  }

  // Reset (if separate reset button)
  if (digitalRead(RESET_BTN) == LOW) {
    delay(50);
    if (digitalRead(RESET_BTN) == LOW) {
      gameRunning = false;
      noTone(BUZZER);
      display.clearDisplay();
      display.setCursor(10, 10);
      display.println("Reaction Test Ready");
      display.display();
      delay(300);
    }
  }
}

/* Non-blocking idle BGM */
void playIdleBGMNonBlocking() {
  unsigned long currentMillis = millis();
  if (!playingNote && currentMillis - lastNoteTime > 80) {
    int note = melody[currentNote];
    int duration = noteDurations[currentNote];
    if (note > 0) tone(BUZZER, note, duration);
    playingNote = true;
    lastNoteTime = currentMillis;
  }
  if (playingNote && (millis() - lastNoteTime >= (unsigned long)noteDurations[currentNote])) {
    noTone(BUZZER);
    playingNote = false;
    lastNoteTime = millis();
    currentNote++;
    if (currentNote >= melodyLength) currentNote = 0;
  }
}

/* Core round: READY (White) -> SET (Yellow) -> GET SET (Green) -> GO (Red) */
void runReactionGame() {
  display.clearDisplay();
  display.setCursor(24, 8);
  display.println("Get Ready...");
  display.display();

  // READY - White
  digitalWrite(WHITE_LED, HIGH);
  tone(BUZZER, 500, 140);
  delay(800);
  digitalWrite(WHITE_LED, LOW);

  // SET - Yellow
  digitalWrite(YELLOW_LED, HIGH);
  tone(BUZZER, 700, 140);
  delay(800);
  digitalWrite(YELLOW_LED, LOW);

  // GET SET - Green
  digitalWrite(GREEN_LED, HIGH);
  tone(BUZZER, 900, 140);
  delay(800);
  digitalWrite(GREEN_LED, LOW);

  // Random wait BEFORE GO — non-blocking check for false starts
  unsigned long waitTime = random(1000, 3000);
  unsigned long t0 = millis();
  while (millis() - t0 < waitTime) {
    // If player presses early -> false start
    if (digitalRead(START_BTN) == LOW) {
      // Debounce and confirm early press
      delay(20);
      if (digitalRead(START_BTN) == LOW) {
        handleFalseStart();
        gameRunning = false;
        return;
      }
    }
    // small yield so loop remains responsive
    delay(5);
  }

  // GO - Red LED + mark start time
  digitalWrite(RED_LED, HIGH);
  display.clearDisplay();
  display.setCursor(52, 10);
  display.println("GO!");
  display.display();
  tone(BUZZER, 1200, 180);
  unsigned long startTime = millis();

  // Wait for player press (active LOW). Debounce on detection.
  while (true) {
    if (digitalRead(START_BTN) == LOW) {
      delay(15); // debounce
      if (digitalRead(START_BTN) == LOW) {
        unsigned long reactionTime = millis() - startTime;
        showResult(reactionTime);
        break;
      }
    }
  }

  // end round visuals
  for (int i = 0; i < 2; i++) {
    digitalWrite(RED_LED, HIGH);
    delay(150);
    digitalWrite(RED_LED, LOW);
    delay(100);
  }

  // ensure all off
  digitalWrite(WHITE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  delay(800);
  gameRunning = false;
}

void handleFalseStart() {
  // Penalty / message for pressing before GO
  noTone(BUZZER);
  display.clearDisplay();
  display.setCursor(10, 6);
  display.println("Too Early!");
  display.setCursor(10, 18);
  display.println("Penalty - Try again");
  display.display();

  // flash red quickly to indicate false start
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 400, 100);
    delay(120);
    digitalWrite(RED_LED, LOW);
    delay(80);
  }
  delay(800);
}

void showResult(unsigned long timeMs) {
  noTone(BUZZER);
  display.clearDisplay();
  display.setCursor(8, 6);
  display.print("Reaction: ");
  display.print(timeMs);
  display.println(" ms");
  display.display();

  // small beep to indicate result
  tone(BUZZER, 1500, 120);
  delay(120);
  noTone(BUZZER);
}
