// ============================================================
//  Purple-Kit  —  BOMB DISPOSAL  v2  (Advanced)
//  Pin map: [row][col]  row0=BOTTOM  row4=TOP
//                       col0=LEFT    col4=RIGHT
// ============================================================

#include <Arduino.h>

// ── MATRIX PIN MAP ───────────────────────────────────────────
const uint8_t MATRIX[5][5] = {
  {29, 38, 39, 48, 49},   // row0 = BOTTOM
  {30, 37, 40, 47, 50},   // row1
  {31, 36, 41, 46, 51},   // row2 = MIDDLE
  {32, 35, 42, 45, 52},   // row3
  {33, 34, 43, 44, 53}    // row4 = TOP
};

// ── BUTTONS ──────────────────────────────────────────────────
const uint8_t BTN_UP    = 28;
const uint8_t BTN_DOWN  = 25;
const uint8_t BTN_LEFT  = 27;
const uint8_t BTN_RIGHT = 26;

// ── BUTTON LEDs (Active-LOW) ─────────────────────────────────
const uint8_t LED_UP    = 21;
const uint8_t LED_DOWN  = 18;
const uint8_t LED_LEFT  = 20;
const uint8_t LED_RIGHT = 19;

// ── OTHER PINS ───────────────────────────────────────────────
const uint8_t RGB_R   = 9;
const uint8_t RGB_G   = 10;
const uint8_t RGB_B   = 11;
const uint8_t BUZZER  = 3;
const uint8_t LDR_PIN = A2;   // LDR — cover it to defuse a bomb

// ── GAME CONSTANTS ───────────────────────────────────────────
const uint8_t  W              = 5;
const uint8_t  H              = 5;
const uint8_t  MAX_LIVES      = 3;
const uint16_t TIME_START     = 5000;  // ms — starting time limit
const uint16_t TIME_MIN       = 900;   // ms — minimum time limit
const uint16_t TIME_DEFUSE    = 200;   // ms — time reduction per defuse
const uint16_t TIME_EXPLODE   = 400;   // ms — time increase after explosion
const uint16_t LDR_THRESH     = 520;   // LDR darkness threshold (cover = defuse)
const uint8_t  MOVE_LEVEL     = 5;     // level at which bomb starts moving
const uint8_t  DOUBLE_LEVEL   = 8;     // level at which 2nd bomb appears
const uint32_t BOMB_MOVE_MS   = 2200;  // ms — bomb movement period
const uint32_t BLINK_ON_MS    = 280;   // ms — player LED visible duration
const uint32_t BLINK_OFF_MS   = 180;   // ms — player LED hidden duration

// ── GAME STATE ───────────────────────────────────────────────
int8_t   playerX    = 2;
int8_t   playerY    = 2;
int8_t   bomb1X     = 0;
int8_t   bomb1Y     = 0;
int8_t   bomb2X     = -1;   // -1 = inactive
int8_t   bomb2Y     = -1;
uint16_t timeLimit  = TIME_START;
uint16_t score      = 0;
uint8_t  lives      = MAX_LIVES;
uint8_t  level      = 1;
uint8_t  combo      = 0;
bool     gameActive = false;
bool     playerVis  = true;   // blink toggle

uint32_t bombTimer     = 0;
uint32_t bombMoveTimer = 0;
uint32_t blinkTimer    = 0;

// Button debounce state
uint8_t  prevUp=HIGH, prevDown=HIGH, prevLeft=HIGH, prevRight=HIGH;
uint32_t tUp=0,       tDown=0,       tLeft=0,       tRight=0;

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for (uint8_t r=0; r<H; r++)
    for (uint8_t c=0; c<W; c++)
      digitalWrite(MATRIX[r][c], LOW);
}

// row 0 = BOTTOM, row 4 = TOP
void ledOn(int8_t col, int8_t row) {
  if (col>=0 && col<(int8_t)W && row>=0 && row<(int8_t)H)
    digitalWrite(MATRIX[row][col], HIGH);
}

void allOn() {
  for (uint8_t r=0; r<H; r++)
    for (uint8_t c=0; c<W; c++)
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

// RGB countdown indicator: blue → green → yellow → red
void updateTimerRGB() {
  uint32_t elapsed = millis() - bombTimer;
  if (elapsed >= timeLimit) { setRGB(1,0,0); return; }
  uint8_t pct = (uint8_t)((timeLimit - elapsed) * 100UL / timeLimit);
  if      (pct > 60) setRGB(0, 0, 1);   // blue   — plenty of time
  else if (pct > 35) setRGB(0, 1, 0);   // green  — hurry up
  else if (pct > 15) setRGB(1, 1, 0);   // yellow — urgent!
  else               setRGB(1, 0, 0);   // red    — critical
}

// ═══════════════════════════════════════════════════════════
//  SOUND
// ═══════════════════════════════════════════════════════════
void playNote(uint16_t hz, uint16_t ms) {
  tone(BUZZER, hz, ms);
  delay(ms + 10);
}

void defuseSound()   { playNote(784,60); playNote(988,60); playNote(1175,70); playNote(1568,160); }
void comboSound()    { playNote(1047,45); playNote(1319,45); playNote(1568,45); playNote(2093,140); }
void explodeSound()  { playNote(220,70); playNote(160,70); playNote(110,70); playNote(65,320); }
void gameOverSound() { playNote(440,160); playNote(349,160); playNote(294,160); playNote(196,480); }
void levelSound()    { playNote(523,55); playNote(659,55); playNote(784,55); playNote(1047,55); playNote(1319,160); }
void startSound()    { playNote(880,120); playNote(1047,180); }

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════
void blinkAll(uint8_t n, uint16_t ms) {
  for (uint8_t i=0; i<n; i++) { allOn(); delay(ms); clearMatrix(); delay(ms); }
}

// Explosion: ripple wave expanding from centre
void explosionAnim() {
  clearMatrix();
  ledOn(2, 2); delay(70); clearMatrix();

  ledOn(1,2); ledOn(3,2); ledOn(2,1); ledOn(2,3);
  delay(70); clearMatrix();

  ledOn(0,2); ledOn(4,2); ledOn(2,0); ledOn(2,4);
  ledOn(1,1); ledOn(3,1); ledOn(1,3); ledOn(3,3);
  delay(70); clearMatrix();

  allOn(); delay(150); clearMatrix();
  blinkAll(3, 90);
}

// Defuse: border frame flash + green RGB
void defuseAnim() {
  setRGB(0, 1, 0);
  clearMatrix();
  for (uint8_t c=0; c<W; c++) { ledOn(c,0); ledOn(c,4); }   // top and bottom rows
  for (uint8_t r=1; r<H-1; r++) { ledOn(0,r); ledOn(4,r); } // side columns
  delay(100); clearMatrix();
  blinkAll(2, 60);
  rgbOff();
}

// Level-up: columns light up left to right
void levelUpAnim() {
  for (uint8_t c=0; c<W; c++) {
    for (uint8_t r=0; r<H; r++) ledOn(c, r);
    delay(70);
  }
  delay(150); clearMatrix();
}

// Show score: 1 LED per point, fills matrix row by row (max 25)
void showScore(uint16_t s) {
  clearMatrix();
  uint8_t show = (uint8_t)min(s, (uint16_t)25);
  for (uint8_t i=0; i<show; i++) ledOn(i%W, i/W);
  delay(2500);
  clearMatrix();
}

// Flash red RGB once per remaining life
void livesAnimation() {
  for (uint8_t i=0; i<lives; i++) {
    setRGB(1,0,0); delay(200); rgbOff(); delay(150);
  }
}

// 3-2-1 countdown bar on middle row
void countdown() {
  const uint16_t notes[3] = {659, 784, 1047};
  for (int8_t i=3; i>0; i--) {
    clearMatrix();
    uint8_t start = (W - (uint8_t)i) / 2;
    for (uint8_t c=0; c<(uint8_t)i; c++) ledOn(start+c, 2);
    if      (i==3) setRGB(1,0,0);
    else if (i==2) setRGB(0,1,0);
    else           setRGB(0,0,1);
    playNote(notes[3-i], 150);
    delay(500);
    rgbOff();
  }
  clearMatrix();
  blinkAll(2, 65);
}

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════

// Spawn bomb at a random cell, avoiding player and another bomb
void spawnBomb(int8_t &bx, int8_t &by,
               int8_t avoidX = -1, int8_t avoidY = -1) {
  do {
    bx = (int8_t)random(0, W);
    by = (int8_t)random(0, H);
  } while (
    (bx == playerX && by == playerY) ||   // never on the player
    (bx == avoidX  && by == avoidY)        // never on the other bomb
  );
}

// Edge-detect button read with 50 ms debounce
bool btnPressed(uint8_t pin, uint8_t &prev, uint32_t &lastT, uint32_t now) {
  uint8_t s = digitalRead(pin);
  bool pressed = (s == LOW && prev == HIGH && now - lastT > 50);
  if (pressed) lastT = now;
  prev = s;
  return pressed;
}

// Mirror button LEDs to button state
void updateButtonLEDs() {
  digitalWrite(LED_UP,    digitalRead(BTN_UP)    == LOW ? LOW : HIGH);
  digitalWrite(LED_DOWN,  digitalRead(BTN_DOWN)  == LOW ? LOW : HIGH);
  digitalWrite(LED_LEFT,  digitalRead(BTN_LEFT)  == LOW ? LOW : HIGH);
  digitalWrite(LED_RIGHT, digitalRead(BTN_RIGHT) == LOW ? LOW : HIGH);
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  clearMatrix(); rgbOff();
  score     = 0;
  lives     = MAX_LIVES;
  level     = 1;
  combo     = 0;
  playerX   = 2;
  playerY   = 2;
  timeLimit = TIME_START;
  spawnBomb(bomb1X, bomb1Y);
  bomb2X    = -1;
  bomb2Y    = -1;
  gameActive = true;
  prevUp = prevDown = prevLeft = prevRight = HIGH;
  countdown();
  startSound();
  bombTimer = bombMoveTimer = blinkTimer = millis();
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  for (uint8_t r=0; r<H; r++)
    for (uint8_t c=0; c<W; c++)
      pinMode(MATRIX[r][c], OUTPUT);

  pinMode(RGB_R,     OUTPUT);
  pinMode(RGB_G,     OUTPUT);
  pinMode(RGB_B,     OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(LED_UP,    OUTPUT); digitalWrite(LED_UP,    HIGH);
  pinMode(LED_DOWN,  OUTPUT); digitalWrite(LED_DOWN,  HIGH);
  pinMode(LED_LEFT,  OUTPUT); digitalWrite(LED_LEFT,  HIGH);
  pinMode(LED_RIGHT, OUTPUT); digitalWrite(LED_RIGHT, HIGH);

  randomSeed(analogRead(A0));
  startGame();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  updateButtonLEDs();
  if (!gameActive) return;

  uint32_t now = millis();

  // ── Player blink timer ───────────────────────────────────
  uint32_t blinkEl = now - blinkTimer;
  if ( playerVis && blinkEl >= BLINK_ON_MS)  { playerVis = false; blinkTimer = now; }
  if (!playerVis && blinkEl >= BLINK_OFF_MS) { playerVis = true;  blinkTimer = now; }

  // ── Directional buttons (debounced) ─────────────────────
  if (btnPressed(BTN_UP,    prevUp,    tUp,    now)) playerY = constrain(playerY+1, 0, H-1);
  if (btnPressed(BTN_DOWN,  prevDown,  tDown,  now)) playerY = constrain(playerY-1, 0, H-1);
  if (btnPressed(BTN_LEFT,  prevLeft,  tLeft,  now)) playerX = constrain(playerX-1, 0, W-1);
  if (btnPressed(BTN_RIGHT, prevRight, tRight, now)) playerX = constrain(playerX+1, 0, W-1);

  // ── Moving bomb (level >= MOVE_LEVEL) ───────────────────
  if (level >= MOVE_LEVEL && now - bombMoveTimer >= BOMB_MOVE_MS) {
    bombMoveTimer = now;
    int8_t nx = constrain(bomb1X + (int8_t)random(-1, 2), 0, W-1);
    int8_t ny = constrain(bomb1Y + (int8_t)random(-1, 2), 0, H-1);
    // BUG FIX: don't land on the player OR on bomb2
    bool hitsPlayer = (nx == playerX && ny == playerY);
    bool hitsBomb2  = (bomb2X >= 0 && nx == bomb2X && ny == bomb2Y);
    if (!hitsPlayer && !hitsBomb2) { bomb1X = nx; bomb1Y = ny; }
  }

  // ── Second bomb (level >= DOUBLE_LEVEL) ─────────────────
  if (level >= DOUBLE_LEVEL && bomb2X < 0)
    spawnBomb(bomb2X, bomb2Y, bomb1X, bomb1Y);

  // ── Bomb explodes (time limit reached) ──────────────────
  if (now - bombTimer >= timeLimit) {
    explosionAnim();
    setRGB(1, 0, 0);
    explodeSound();
    rgbOff();

    combo = 0;
    lives--;
    timeLimit = (uint16_t)constrain(
      (int16_t)timeLimit + TIME_EXPLODE, TIME_MIN, TIME_START
    );

    if (lives == 0) {
      gameActive = false;
      gameOverSound();
      blinkAll(5, 130);
      showScore(score);
      delay(500);
      startGame();
      return;
    }

    livesAnimation();
    spawnBomb(bomb1X, bomb1Y);
    bomb2X = -1; bomb2Y = -1;
    bombTimer = bombMoveTimer = millis();
    return;
  }

  // ── Player on bomb + LDR covered → DEFUSE ───────────────
  bool onBomb1 = (playerX == bomb1X && playerY == bomb1Y);
  bool onBomb2 = (bomb2X >= 0 && playerX == bomb2X && playerY == bomb2Y);

  if ((onBomb1 || onBomb2) && analogRead(LDR_PIN) > LDR_THRESH) {
    score++;
    combo++;

    // Check for level-up (every 5 defuses)
    uint8_t newLevel = 1 + (uint8_t)(score / 5);
    if (newLevel > level) {
      level = newLevel;
      levelSound();
      levelUpAnim();
    }

    // Combo bonus at 3+ consecutive defuses
    if (combo >= 3) {
      comboSound();
      score++;   // +1 bonus point
    } else {
      defuseSound();
    }

    defuseAnim();

    // Speed up (minimum TIME_MIN)
    timeLimit = (uint16_t)constrain(
      (int16_t)timeLimit - TIME_DEFUSE, TIME_MIN, TIME_START
    );

    // Respawn bomb(s)
    spawnBomb(bomb1X, bomb1Y);
    if (level >= DOUBLE_LEVEL)
      spawnBomb(bomb2X, bomb2Y, bomb1X, bomb1Y);
    else
      bomb2X = -1;

    bombTimer = bombMoveTimer = millis();
    return;
  }

  // ── RGB countdown indicator ──────────────────────────────
  updateTimerRGB();

  // ── Render frame ─────────────────────────────────────────
  clearMatrix();

  // Player blinks so they're distinguishable from bombs
  if (playerVis) ledOn(playerX, playerY);

  // Draw bombs only if player is not standing on them
  // (player blink takes visual priority)
  if (!onBomb1) ledOn(bomb1X, bomb1Y);
  if (bomb2X >= 0 && !onBomb2) ledOn(bomb2X, bomb2Y);

  delay(15);
}
