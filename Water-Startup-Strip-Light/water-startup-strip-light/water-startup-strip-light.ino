// ESP32 LED Flow with Tilt-controlled "Pouring Water" Effect
#define LED1 13
#define LED2 12
#define LED3 14
#define LED4 27
#define LED5 26
#define TILT 25   // tilt switch input pin

const int leds[] = {LED1, LED2, LED3, LED4, LED5};
const int numLeds = sizeof(leds) / sizeof(leds[0]);

const int stepDelay = 200;   // ms per step
const int repeatCycles = 3;  // normal flow cycles before fill

void setup() {
  for (int i = 0; i < numLeds; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }
  pinMode(TILT, INPUT_PULLUP);  // tilt switch with pull-up
}

void loop() {
  bool tiltUp = digitalRead(TILT) == HIGH; // adjust depending on wiring

  if (tiltUp) {
    // --- Normal flow phase (repeat 3x) ---
    for (int cycle = 0; cycle < repeatCycles; cycle++) {
      forwardFlowClear();
    }

    // --- Pouring/fill phase ---
    pouringFill();

  } else {
    // TODO: define DOWN behavior later if needed
    reverseFlowClear(); // placeholder
  }

  delay(500); // pause between loops
}

// Normal forward flow: clears each step
void forwardFlowClear() {
  clearAll();
  digitalWrite(leds[0], HIGH); delay(stepDelay);

  for (int i = 0; i < numLeds - 1; i++) {
    clearAll();
    digitalWrite(leds[i], HIGH);
    digitalWrite(leds[i + 1], HIGH);
    delay(stepDelay);
  }

  clearAll();
  digitalWrite(leds[numLeds - 1], HIGH); delay(stepDelay);
}

// Pouring water fill: keeps LEDs latched ON
void pouringFill() {
  clearAll();
  for (int i = 0; i < numLeds; i++) {
    // keep all previous ON
    for (int j = 0; j <= i; j++) {
      digitalWrite(leds[j], HIGH);
    }
    delay(stepDelay);
  }
}

// Placeholder for tilt down
void reverseFlowClear() {
  for (int i = numLeds - 1; i >= 0; i--) {
    clearAll();
    digitalWrite(leds[i], HIGH);
    if (i > 0) digitalWrite(leds[i - 1], HIGH);
    delay(stepDelay);
  }
}

void clearAll() {
  for (int i = 0; i < numLeds; i++) digitalWrite(leds[i], LOW);
}
