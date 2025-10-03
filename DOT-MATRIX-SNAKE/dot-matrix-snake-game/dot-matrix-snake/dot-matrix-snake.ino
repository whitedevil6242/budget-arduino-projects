#include <MD_MAX72xx.h>
#include <SPI.h>

// MAX7219 setup
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 1
#define DATA_PIN   11
#define CS_PIN     10
#define CLK_PIN    13

MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN);

// Joystick pins
#define JOY_X A0
#define JOY_Y A1
#define JOY_SW 2

// Buzzer
#define BUZZER 3

// Snake game variables
int snakeX[64], snakeY[64];
int snakeLength = 3;
int dirX = 1, dirY = 0;
int foodX, foodY;
unsigned long lastMove = 0;
int speed = 300; // lower = faster

// --- SOUND VARIABLES ---
bool bgmEnabled = true;
unsigned long lastNoteTime = 0;
int currentNote = 0;

// Simple looping background melody
int bgmMelody[] = { 262, 330, 392, 523, 392, 330 }; // C, E, G, C5, G, E
int bgmDurations[] = { 300, 300, 300, 300, 300, 300 };
int bgmLength = sizeof(bgmMelody)/sizeof(bgmMelody[0]);

// --- SOUND FUNCTIONS ---
void beepEffect(int freq = 1000, int duration = 80) {
  tone(BUZZER, freq, duration);
  delay(duration + 10);
}

void playEatSound() {
  tone(BUZZER, 1200, 80);
  delay(100);
  tone(BUZZER, 1500, 80);
  delay(100);
  noTone(BUZZER);
}

void playGameOverSound() {
  bgmEnabled = false; // stop bgm
  tone(BUZZER, 300, 600);
  delay(650);
  tone(BUZZER, 200, 800);
  delay(850);
  noTone(BUZZER);
  bgmEnabled = true; // re-enable bgm for restart
}

void playStartMelody() {
  int melody[] = { 523, 659, 784, 1047 }; // C5, E5, G5, C6
  int noteDurations[] = { 200, 200, 200, 300 };

  for (unsigned int i = 0; i < sizeof(melody)/sizeof(melody[0]); i++) {
    tone(BUZZER, melody[i], noteDurations[i]);
    delay(noteDurations[i] + 20);
  }
  noTone(BUZZER);
}

void playBackgroundMusic() {
  if (!bgmEnabled) return;

  unsigned long now = millis();
  if (now - lastNoteTime >= (unsigned long)bgmDurations[currentNote]) {
    // play next note
    tone(BUZZER, bgmMelody[currentNote], bgmDurations[currentNote]);
    lastNoteTime = now;
    currentNote++;
    if (currentNote >= bgmLength) currentNote = 0; // loop back
  }
}
// -----------------------

void setup() {
  matrix.begin();
  matrix.clear();

  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // Initial snake position
  snakeX[0] = 3; snakeY[0] = 4;
  snakeX[1] = 2; snakeY[1] = 4;
  snakeX[2] = 1; snakeY[2] = 4;

  randomSeed(analogRead(0));
  spawnFood();

  // Play start melody
  playStartMelody();
}

void loop() {
  readJoystick();

  if (millis() - lastMove > speed) {
    lastMove = millis();
    moveSnake();
    drawGame();
  }

  // play background tune continuously
  playBackgroundMusic();
}

void readJoystick() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  if (x < 400 && dirX == 0) { dirX =  1; dirY = 0; } // RIGHT (reversed)
  if (x > 600 && dirX == 0) { dirX = -1; dirY = 0; } // LEFT (reversed)
  if (y < 400 && dirY == 0) { dirX =  0; dirY = -1; } // UP
  if (y > 600 && dirY == 0) { dirX =  0; dirY =  1; } // DOWN
}

void moveSnake() {
  // shift snake
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }

  snakeX[0] += dirX;
  snakeY[0] += dirY;

  // Wrap around edges
  if (snakeX[0] < 0) snakeX[0] = 7;
  if (snakeX[0] > 7) snakeX[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = 7;
  if (snakeY[0] > 7) snakeY[0] = 0;

  // Check food
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    playEatSound();   // 🎵 play sound when food eaten
    spawnFood();
  }

  // Check self collision
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver();
    }
  }
}

void drawGame() {
  matrix.clear();
  
  // Draw snake
  for (int i = 0; i < snakeLength; i++) {
    matrix.setPoint(snakeY[i], snakeX[i], true);
  }

  // Draw food
  matrix.setPoint(foodY, foodX, true);
}

void spawnFood() {
  foodX = random(0, 8);
  foodY = random(0, 8);
}

void gameOver() {
  playGameOverSound();   // 🎵 game over tune
  delay(2000);

  // Reset snake
  snakeLength = 3;
  snakeX[0] = 3; snakeY[0] = 4;
  snakeX[1] = 2; snakeY[1] = 4;
  snakeX[2] = 1; snakeY[2] = 4;
  dirX = 1; dirY = 0;
  spawnFood();
}
