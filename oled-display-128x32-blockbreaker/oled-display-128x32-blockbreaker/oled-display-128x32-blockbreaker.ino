#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick
#define JOY_X A0
#define JOY_Y A1
#define JOY_SW 2

// Buzzers
#define BUZZER_PASSIVE 3  // background music
#define BUZZER_ACTIVE 4   // hit + win/lose sounds

// Game variables
int paddleX;
const int paddleW = 18;
const int paddleH = 3;

int ballX, ballY;
int ballDX = 1, ballDY = -1;
const int ballSize = 2;

const int blockRows = 4;   // more rows
const int blockCols = 16;  // more columns
const int blockW = 8;
const int blockH = 4;
bool blocks[blockRows][blockCols];

bool gameOver = false;
bool gameWin = false;

void setup() {
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(BUZZER_ACTIVE, OUTPUT);
  pinMode(BUZZER_PASSIVE, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  resetGame();
}

// Reset game
void resetGame() {
  paddleX = (SCREEN_WIDTH - paddleW) / 2;
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT - paddleH - ballSize - 1;
  ballDX = 1;
  ballDY = -1;

  for (int r = 0; r < blockRows; r++) {
    for (int c = 0; c < blockCols; c++) {
      blocks[r][c] = true;
    }
  }

  gameOver = false;
  gameWin = false;
  noTone(BUZZER_ACTIVE);
  noTone(BUZZER_PASSIVE);
}

// Hit sound
void hitBeep() {
  digitalWrite(BUZZER_ACTIVE, HIGH);
  delay(30);
  digitalWrite(BUZZER_ACTIVE, LOW);
}

// Game over sound
void gameOverSound() {
  int tones[] = { 440, 392, 349, 330 };
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_ACTIVE, tones[i], 200);
    delay(200);
  }
  noTone(BUZZER_ACTIVE);
}

// Win sound
void winSound() {
  int tones[] = { 330, 392, 440, 523 };
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_ACTIVE, tones[i], 200);
    delay(200);
  }
  noTone(BUZZER_ACTIVE);
}

// Background music
void playBGM() {
  if (gameOver || gameWin) {
    noTone(BUZZER_PASSIVE);
    return;
  }

  static unsigned long lastNote = 0;
  static int noteIndex = 0;

  int melody[] = { 262, 294, 330, 349, 392, 440, 494, 523 }; // C scale
  int noteDuration = 200;

  if (millis() - lastNote > noteDuration) {
    tone(BUZZER_PASSIVE, melody[noteIndex], noteDuration);
    noteIndex = (noteIndex + 1) % 8;
    lastNote = millis();
  }
}

bool allBlocksCleared() {
  for (int r = 0; r < blockRows; r++) {
    for (int c = 0; c < blockCols; c++) {
      if (blocks[r][c]) return false;
    }
  }
  return true;
}

void loop() {
  playBGM();

  if (!gameOver && !gameWin) {
    // Joystick input (Y axis controls left-right)
    int joyY = analogRead(JOY_Y);
    if (joyY < 400) {
      paddleX -= 2;  // up = left
    } else if (joyY > 600) {
      paddleX += 2;  // down = right
    }

    if (paddleX < 0) paddleX = 0;
    if (paddleX > SCREEN_WIDTH - paddleW) paddleX = SCREEN_WIDTH - paddleW;

    // Move ball
    ballX += ballDX;
    ballY += ballDY;

    // Wall collision
    if (ballX <= 0 || ballX >= SCREEN_WIDTH - ballSize) {
      ballDX = -ballDX;
      hitBeep();
    }
    if (ballY <= 0) {
      ballDY = -ballDY;
      hitBeep();
    }

    // Paddle collision
    if (ballY >= SCREEN_HEIGHT - paddleH - ballSize &&
        ballX + ballSize >= paddleX &&
        ballX <= paddleX + paddleW) {
      ballDY = -ballDY;
      hitBeep();
    }

    // Block collision
    for (int r = 0; r < blockRows; r++) {
      for (int c = 0; c < blockCols; c++) {
        if (blocks[r][c]) {
          int bx = c * blockW;
          int by = r * blockH;
          if (ballX + ballSize > bx && ballX < bx + blockW &&
              ballY + ballSize > by && ballY < by + blockH) {
            blocks[r][c] = false;
            ballDY = -ballDY;
            hitBeep();
          }
        }
      }
    }

    // Check win
    if (allBlocksCleared()) {
      gameWin = true;
      winSound();
    }

    // Check game over
    if (ballY > SCREEN_HEIGHT) {
      gameOver = true;
      gameOverSound();
    }

    // Draw
    display.clearDisplay();

    // Draw blocks
    for (int r = 0; r < blockRows; r++) {
      for (int c = 0; c < blockCols; c++) {
        if (blocks[r][c]) {
          display.fillRect(c * blockW, r * blockH, blockW - 1, blockH - 1, SSD1306_WHITE);
        }
      }
    }

    // Draw paddle
    display.fillRect(paddleX, SCREEN_HEIGHT - paddleH, paddleW, paddleH, SSD1306_WHITE);

    // Draw ball
    display.fillRect(ballX, ballY, ballSize, ballSize, SSD1306_WHITE);

    display.display();
    delay(10);

  } else {
    // Game Over / Win screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    if (gameOver) {
      display.setCursor(30, 10);
      display.println("GAME OVER");
    } else if (gameWin) {
      display.setCursor(30, 10);
      display.println("YOU WIN!");
    }

    display.setCursor(20, 22);
    display.println("Press SW to restart");
    display.display();

    // Restart on joystick press
    if (digitalRead(JOY_SW) == LOW) {
      resetGame();
    }
  }
}
