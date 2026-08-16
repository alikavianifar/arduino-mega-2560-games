// ============================================================
//  Purple-Kit  —  STAR CATCHER  v2
//  Pin mapping: [row][col]  row0=BOTTOM  row4=TOP
// ============================================================

#include <Arduino.h>

// ── MATRIX PIN MAP (same as Snake reference) ─────────────────
const uint8_t MATRIX[5][5] = {
  {29, 38, 39, 48, 49},   // row0 = BOTTOM  (basket drawn here)
  {30, 37, 40, 47, 50},   // row1
  {31, 36, 41, 46, 51},   // row2 = MIDDLE
  {32, 35, 42, 45, 52},   // row3
  {33, 34, 43, 44, 53}    // row4 = TOP     (star spawns here)
};

// ── PINS ─────────────────────────────────────────────────────
const uint8_t RGB_R       = 9;
const uint8_t RGB_G       = 10;
const uint8_t RGB_B       = 11;
const uint8_t BUZZER      = 3;
const uint8_t POT_PIN     = A1;
const uint8_t BTN_PAUSE   = 25;   // DOWN button — pause / resume
const uint8_t BTN_RESTART = 28;   // UP button   — restart game
const uint8_t LED_PAUSE   = 18;   // DOWN button LED — lit when paused (active LOW)

// ── GAME CONSTANTS ───────────────────────────────────────────
const uint8_t  W               = 5;
const uint8_t  H               = 5;
const uint8_t  MAX_LIVES       = 3;
const uint16_t SPEED_START     = 600;   // ms — starting fall interval (slow)
const uint16_t SPEED_MIN       = 80;    // ms — fastest possible fall interval
const uint16_t SPEED_CATCH     = 25;    // ms reduction per successful catch
const uint16_t SPEED_MISS      = 60;    // ms increase (slowdown) per miss
const uint16_t SPEED_AUTO      = 15;    // ms reduction per automatic speed-up tick
const uint32_t SPEED_INTERVAL  = 8000;  // ms — automatic speed-up period
const uint8_t  BASKET_W        = 2;     // basket width in columns

// ── GAME STATE ───────────────────────────────────────────────
int8_t   starX       = 2;
int8_t   starY       = H - 1;    // starts at row4 (TOP)
int8_t   basketX     = 1;
uint16_t fallSpeed   = SPEED_START;
uint16_t score       = 0;
uint8_t  lives       = MAX_LIVES;
bool     gameActive  = false;
bool     paused      = false;
uint32_t lastFall    = 0;
uint32_t lastSpeedUp = 0;

// Button debounce state
uint8_t  btn1Prev = HIGH,  btn2Prev = HIGH;
uint32_t btn1Time = 0,     btn2Time = 0;

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for (uint8_t r = 0; r < H; r++)
    for (uint8_t c = 0; c < W; c++)
      digitalWrite(MATRIX[r][c], LOW);
}

// col 0=LEFT  col 4=RIGHT  |  row 0=BOTTOM  row 4=TOP
void ledOn(uint8_t col, uint8_t row) {
  if (col < W && row < H)
    digitalWrite(MATRIX[row][col], HIGH);
}

void allOn() {
  for (uint8_t r = 0; r < H; r++)
    for (uint8_t c = 0; c < W; c++)
      digitalWrite(MATRIX[r][c], HIGH);
}

// ═══════════════════════════════════════════════════════════
//  RGB
// ═══════════════════════════════════════════════════════════
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}
void rgbOff() { setRGB(0, 0, 0); }

// RGB reflects fall speed: slow=blue, medium=green, fast=red
void updateSpeedRGB() {
  uint16_t range     = SPEED_START - SPEED_MIN;
  uint16_t remaining = fallSpeed   - SPEED_MIN;
  if      (remaining > range * 2 / 3) setRGB(0, 0, 1);   // blue  = slow
  else if (remaining > range / 3)     setRGB(0, 1, 0);   // green = medium
  else                                setRGB(1, 0, 0);   // red   = fast
}

// ═══════════════════════════════════════════════════════════
//  SOUND
// ═══════════════════════════════════════════════════════════
void playNote(uint16_t hz, uint16_t ms) {
  tone(BUZZER, hz, ms);
  delay(ms + 10);
}

// Ascending arpeggio played on a successful catch
void catchMelody() {
  playNote(523, 80); playNote(659, 80);
  playNote(784, 80); playNote(1047, 200);
}

// Descending tones played on a miss
void missMelody() {
  playNote(300, 100); playNote(200, 100); playNote(120, 300);
}

// Sad fanfare played on game over
void gameOverMelody() {
  playNote(440, 150); playNote(349, 150);
  playNote(294, 150); playNote(220, 500);
}

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════
void blinkAll(uint8_t times, uint16_t ms) {
  for (uint8_t i = 0; i < times; i++) {
    allOn(); delay(ms);
    clearMatrix(); delay(ms);
  }
}

// Show score as lit LEDs (max 25, fills matrix row by row)
void showScore(uint16_t s) {
  clearMatrix();
  uint8_t count = (uint8_t)min(s, (uint16_t)25);
  for (uint8_t i = 0; i < count; i++)
    ledOn(i % W, i / W);
  delay(2500);
  clearMatrix();
}

// Blink RGB red once per remaining life
void livesAnimation() {
  for (uint8_t i = 0; i < lives; i++) {
    setRGB(1, 0, 0); delay(200);
    rgbOff();         delay(150);
  }
}

// 3-2-1 countdown: shrinking bar of LEDs on middle row
void countdown() {
  const uint16_t notes[3] = {659, 784, 1047};
  for (int8_t i = 3; i > 0; i--) {
    clearMatrix();
    uint8_t start = (W - (uint8_t)i) / 2;
    for (uint8_t c = 0; c < (uint8_t)i; c++)
      ledOn(start + c, 2);                    // middle row (row2)
    if      (i == 3) setRGB(1, 0, 0);
    else if (i == 2) setRGB(0, 1, 0);
    else             setRGB(0, 0, 1);
    playNote(notes[3 - i], 150);
    delay(500);
    rgbOff();
  }
  clearMatrix();
  blinkAll(2, 70);   // "GO!" flash
}

// ═══════════════════════════════════════════════════════════
//  DRAW
// ═══════════════════════════════════════════════════════════
void drawFrame() {
  clearMatrix();
  ledOn((uint8_t)starX, (uint8_t)starY);     // falling star
  for (uint8_t i = 0; i < BASKET_W; i++)
    ledOn((uint8_t)(basketX + i), 0);         // basket at row0 (BOTTOM)
}

// ═══════════════════════════════════════════════════════════
//  BUTTON (rising-edge detect with 50 ms debounce)
// ═══════════════════════════════════════════════════════════
bool buttonPressed(uint8_t pin, uint8_t &prev, uint32_t &lastT) {
  uint8_t cur = digitalRead(pin);
  if (cur == LOW && prev == HIGH && millis() - lastT > 50) {
    prev = LOW; lastT = millis(); return true;
  }
  if (cur == HIGH) prev = HIGH;
  return false;
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  clearMatrix(); rgbOff();
  score       = 0;
  lives       = MAX_LIVES;
  fallSpeed   = SPEED_START;
  starX       = (int8_t)random(0, W);
  starY       = H - 1;
  basketX     = (W - BASKET_W) / 2;
  gameActive  = true;
  paused      = false;
  // Reset debounce to prevent phantom button triggers after blocking calls
  btn1Prev = btn2Prev = HIGH;
  btn1Time = btn2Time = millis();
  digitalWrite(LED_PAUSE, HIGH);   // pause LED off (active-low)
  countdown();
  lastFall    = millis();
  lastSpeedUp = millis();
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  for (uint8_t r = 0; r < H; r++)
    for (uint8_t c = 0; c < W; c++)
      pinMode(MATRIX[r][c], OUTPUT);

  pinMode(RGB_R,       OUTPUT);
  pinMode(RGB_G,       OUTPUT);
  pinMode(RGB_B,       OUTPUT);
  pinMode(BUZZER,      OUTPUT);
  pinMode(BTN_PAUSE,   INPUT_PULLUP);
  pinMode(BTN_RESTART, INPUT_PULLUP);
  pinMode(LED_PAUSE,   OUTPUT); digitalWrite(LED_PAUSE, HIGH);  // start off

  randomSeed(analogRead(A0));
  startGame();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {

  // ── Pause / Resume (BTN_PAUSE) ─────────────────────────
  if (buttonPressed(BTN_PAUSE, btn1Prev, btn1Time) && gameActive) {
    paused = !paused;
    if (paused) {
      setRGB(0, 0, 1);
      digitalWrite(LED_PAUSE, LOW);    // pause LED on (active-low)
      tone(BUZZER, 880, 100); delay(120);
    } else {
      rgbOff();
      digitalWrite(LED_PAUSE, HIGH);   // pause LED off
      tone(BUZZER, 1047, 100); delay(120);
      // Resync timers so falling resumes from now, not from before the pause
      lastFall    = millis();
      lastSpeedUp = millis();
    }
  }

  // ── Restart (BTN_RESTART) ───────────────────────────────
  if (buttonPressed(BTN_RESTART, btn2Prev, btn2Time)) {
    startGame();
    return;
  }

  if (!gameActive || paused) return;

  // ── Basket Position: Potentiometer ─────────────────────
  // Map to (W-BASKET_W+1) then constrain → equal zones across full pot range
  // e.g. W=5, BASKET_W=2: map to [0,4], constrain to [0,3]
  basketX = (int8_t)constrain(
    map(analogRead(POT_PIN), 0, 1023, 0, W - BASKET_W + 1),
    0, W - BASKET_W
  );

  // ── Automatic Speed-Up (time-based) ────────────────────
  // Every SPEED_INTERVAL ms the fall speed increases by SPEED_AUTO ms
  if (millis() - lastSpeedUp >= SPEED_INTERVAL) {
    lastSpeedUp = millis();
    if (fallSpeed > SPEED_MIN) {
      fallSpeed = (uint16_t)constrain(
        (int16_t)fallSpeed - SPEED_AUTO,
        SPEED_MIN, SPEED_START
      );
    }
  }

  // ── Star Fall: row4 → row0 ──────────────────────────────
  uint32_t now = millis();
  if (now - lastFall >= fallSpeed) {
    lastFall = now;
    starY--;   // 4 → 3 → 2 → 1 → 0

    if (starY == 0) {
      // Both star and basket are at row0 — check alignment
      bool caught = false;
      for (uint8_t i = 0; i < BASKET_W; i++)
        if (starX == basketX + (int8_t)i) { caught = true; break; }

      if (caught) {
        score++;
        fallSpeed = (uint16_t)constrain(
          (int16_t)fallSpeed - SPEED_CATCH, SPEED_MIN, SPEED_START
        );
        catchMelody();
        setRGB(0, 1, 0); blinkAll(1, 60); rgbOff();

      } else {
        // Missed the star
        lives--;
        missMelody();
        setRGB(1, 0, 0); delay(400); rgbOff();

        if (lives == 0) {
          // Game over
          gameActive = false;
          gameOverMelody();
          blinkAll(4, 100);
          showScore(score);
          delay(1000);
          startGame();
          return;
        }

        // Slight slowdown as a grace period after a miss
        fallSpeed = (uint16_t)constrain(
          (int16_t)fallSpeed + SPEED_MISS, SPEED_MIN, SPEED_START
        );
        livesAnimation();
      }

      // Spawn new star at the top
      starX       = (int8_t)random(0, W);
      starY       = H - 1;
      lastFall    = millis();
      lastSpeedUp = millis();   // reset auto-speedup timer after each catch/miss
    }
  }

  // ── Render Frame ────────────────────────────────────────
  drawFrame();
  updateSpeedRGB();

  delayMicroseconds(500);
}
