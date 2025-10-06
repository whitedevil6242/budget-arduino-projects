#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// LED pins
#define RED_LED 2
#define YELLOW_LED 3
#define GREEN_LED 4
#define WHITE_LED 5

// Buzzer
#define BUZZER 6

// Buttons
#define REACTION_BTN 7
#define RESET_BTN 8

// Game variables
unsigned long startTime = 0;
unsigned long reactionTime = 0;
bool gameActive = false;
bool waitingForGo = false;

void setup() {
  // Pin setup
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(REACTION_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is your OLED address
    for (;;); // If fails, freeze
  }

  // Welcome screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println(F("Reaction Timer Game"));
  display.display();
  delay(1500);
  showMessage("Press Button to Start");
}

void loop() {
  // Start game
  if (!gameActive && digitalRead(REACTION_BTN) == LOW) {
    delay(200); // debounce
    startSequence();
  }

  // Record reaction time
  if (gameActive && !waitingForGo && digitalRead(REACTION_BTN) == LOW) {
    reactionTime = millis() - startTime;
    showResult(reactionTime);
    gameActive = false;
  }

  // Reset
  if (digitalRead(RESET_BTN) == LOW) {
    delay(200);
    resetGame();
  }
}

// ========== FUNCTIONS ==========

void startSequence() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Get Ready...");
  display.display();

  // Ready light
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER, 500, 200);
  delay(1000);
  digitalWrite(RED_LED, LOW);

  // Set light
  digitalWrite(YELLOW_LED, HIGH);
  tone(BUZZER, 700, 200);
  delay(1000);
  digitalWrite(YELLOW_LED, LOW);

  // Random wait before Go
  waitingForGo = true;
  delay(random(1000, 3000));

  // Go signal
  digitalWrite(GREEN_LED, HIGH);
  tone(BUZZER, 1000, 200);
  startTime = millis();
  waitingForGo = false;
  gameActive = true;

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("GO!");
  display.display();
}

void showResult(unsigned long time) {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(WHITE_LED, HIGH);
  tone(BUZZER, 1500, 200);
  delay(200);
  digitalWrite(WHITE_LED, LOW);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Reaction: ");
  display.print(time);
  display.println(" ms");
  display.display();
}

void resetGame() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(WHITE_LED, LOW);
  noTone(BUZZER);
  gameActive = false;
  waitingForGo = false;
  showMessage("Press Button to Start");
}

void showMessage(const char* msg) {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println(msg);
  display.display();
}
