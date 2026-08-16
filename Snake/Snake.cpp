// ============================================================
//  Purple-Kit  —  SNAKE
//  Pin map: [row][col]  row0=BOTTOM  row4=TOP
//                       col0=LEFT    col4=RIGHT
// ============================================================

#include <Arduino.h>

// ── MATRIX PIN MAP ───────────────────────────────────────────
const int MATRIX[5][5] = {
  {29, 38, 39, 48, 49},   // row0 = BOTTOM
  {30, 37, 40, 47, 50},   // row1
  {31, 36, 41, 46, 51},   // row2 = MIDDLE
  {32, 35, 42, 45, 52},   // row3
  {33, 34, 43, 44, 53}    // row4 = TOP
};

// ── PINS ─────────────────────────────────────────────────────
const int BTN_UP    = 28;   // Blue
const int BTN_DOWN  = 25;   // Green
const int BTN_LEFT  = 27;   // White
const int BTN_RIGHT = 26;   // Red

const int LED_BTN_UP    = 21;
const int LED_BTN_DOWN  = 18;
const int LED_BTN_LEFT  = 20;
const int LED_BTN_RIGHT = 19;

const int BUZZER  = 3;
const int RGB_R   = 9, RGB_G = 10, RGB_B = 11;
const int POT_PIN = A1;

// Active-Low button LEDs
const int LED_BTN_ON  = LOW;
const int LED_BTN_OFF = HIGH;

// ── GAME CONSTANTS ───────────────────────────────────────────
const int  GRID          = 5;
const long MIN_INTERVAL  = 100;   // ms — absolute speed floor
const int  FOOD_BLINK_MS = 300;   // ms — food LED toggle period

// ── GAME STATE ───────────────────────────────────────────────
int  snakeX[25], snakeY[25];
int  snakeLen;
int  dirX, dirY;
int  foodX, foodY;
bool foodVisible = true;

unsigned long moveInterval;
unsigned long lastMoveTime;
unsigned long lastFoodBlink;
bool gameOver;

bool prevUp = HIGH, prevDown = HIGH, prevLeft = HIGH, prevRight = HIGH;

// ═══════════════════════════════════════════════════════════
//  RGB HELPER
// ═══════════════════════════════════════════════════════════
void setColor(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}

// ═══════════════════════════════════════════════════════════
//  FOOD SPAWN
// ═══════════════════════════════════════════════════════════
void spawnFood() {
  bool valid;
  do {
    valid = true;
    foodX = random(0, GRID);
    foodY = random(0, GRID);
    for (int i = 0; i < snakeLen; i++)
      if (snakeX[i] == foodX && snakeY[i] == foodY) { valid = false; break; }
  } while (!valid);
  foodVisible = true;   // ensure food is immediately visible after spawn
}

// ═══════════════════════════════════════════════════════════
//  BUTTON FEEDBACK (LEDs mirror button state)
// ═══════════════════════════════════════════════════════════
void updateButtonFeedback() {
  digitalWrite(LED_BTN_UP,    digitalRead(BTN_UP)    == LOW ? LED_BTN_ON : LED_BTN_OFF);
  digitalWrite(LED_BTN_DOWN,  digitalRead(BTN_DOWN)  == LOW ? LED_BTN_ON : LED_BTN_OFF);
  digitalWrite(LED_BTN_LEFT,  digitalRead(BTN_LEFT)  == LOW ? LED_BTN_ON : LED_BTN_OFF);
  digitalWrite(LED_BTN_RIGHT, digitalRead(BTN_RIGHT) == LOW ? LED_BTN_ON : LED_BTN_OFF);
}

// ═══════════════════════════════════════════════════════════
//  SPEED CONTROL (POTENTIOMETER)
// ═══════════════════════════════════════════════════════════
void updateSpeedFromPot() {
  int potVal      = analogRead(POT_PIN);
  int baseInterval = map(potVal, 0, 1023, 800, 150);   // 800ms (slow) → 150ms (fast)
  int bonus       = (snakeLen - 2) * 20;               // longer snake = slightly faster

  // BUG FIX: signed arithmetic before assigning to unsigned long.
  // Without this, a long snake + fast pot setting could make (baseInterval-bonus)
  // negative, wrapping around to an enormous unsigned value and freezing the game.
  long interval = (long)baseInterval - bonus;
  if (interval < MIN_INTERVAL) interval = MIN_INTERVAL;
  moveInterval = (unsigned long)interval;
}

// ═══════════════════════════════════════════════════════════
//  DRAW MATRIX
// ═══════════════════════════════════════════════════════════
void drawMatrix() {
  // Clear all LEDs
  for (int r = 0; r < GRID; r++)
    for (int c = 0; c < GRID; c++)
      digitalWrite(MATRIX[r][c], LOW);

  // Draw snake body
  for (int i = 0; i < snakeLen; i++)
    digitalWrite(MATRIX[snakeY[i]][snakeX[i]], HIGH);

  // Draw food — blinks so it's distinguishable from the snake body
  if (foodVisible)
    digitalWrite(MATRIX[foodY][foodX], HIGH);
}

// ═══════════════════════════════════════════════════════════
//  INPUT (edge-detect: one direction change per button press)
// ═══════════════════════════════════════════════════════════
void readInput() {
  bool up    = digitalRead(BTN_UP);
  bool down  = digitalRead(BTN_DOWN);
  bool left  = digitalRead(BTN_LEFT);
  bool right = digitalRead(BTN_RIGHT);

  // row0=BOTTOM row4=TOP → moving "up" means Y increases
  if (up    == LOW && prevUp    == HIGH && dirY != -1) { dirX =  0; dirY =  1; }
  if (down  == LOW && prevDown  == HIGH && dirY !=  1) { dirX =  0; dirY = -1; }
  if (left  == LOW && prevLeft  == HIGH && dirX !=  1) { dirX = -1; dirY =  0; }
  if (right == LOW && prevRight == HIGH && dirX != -1) { dirX =  1; dirY =  0; }

  prevUp = up; prevDown = down; prevLeft = left; prevRight = right;
}

// ═══════════════════════════════════════════════════════════
//  GAME OVER ANIMATION
// ═══════════════════════════════════════════════════════════
void triggerGameOver() {
  gameOver = true;
  tone(BUZZER, 250, 400);
  setColor(true, false, false);   // red
  delay(700);
  setColor(false, false, false);
  delay(300);
  // gameOver is reset by startGame()
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  snakeLen  = 2;
  snakeX[0] = 3; snakeY[0] = 2;   // head — center-right
  snakeX[1] = 2; snakeY[1] = 2;   // tail — one cell to the left
  dirX = 1; dirY = 0;              // moving right
  gameOver  = false;
  spawnFood();
  lastMoveTime  = millis();
  lastFoodBlink = millis();
  drawMatrix();
  setColor(false, false, false);
  tone(BUZZER, 1000, 200);
}

// ═══════════════════════════════════════════════════════════
//  GAME STEP (advance snake one cell)
// ═══════════════════════════════════════════════════════════
void stepGame() {
  int newX = snakeX[0] + dirX;
  int newY = snakeY[0] + dirY;

  // Wrap-around edges
  if (newX <  0)    newX = GRID - 1;
  if (newX >= GRID) newX = 0;
  if (newY <  0)    newY = GRID - 1;
  if (newY >= GRID) newY = 0;

  // Self-collision check — BUG FIX: exclude the tail (index snakeLen-1).
  // The tail moves away this same step, so the head is allowed to enter that cell.
  // Old code used (i < snakeLen) which prevented a legitimate "tail-chase" move.
  for (int i = 0; i < snakeLen - 1; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      triggerGameOver();
      startGame();
      return;
    }
  }

  bool ateFood = (newX == foodX && newY == foodY);

  // Shift body segments forward
  for (int i = snakeLen; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  snakeX[0] = newX;
  snakeY[0] = newY;

  if (ateFood) {
    snakeLen++;
    tone(BUZZER, 1500, 80);
    setColor(false, true, false);   // green flash
    delay(150);
    setColor(false, false, false);
    spawnFood();
  }

  drawMatrix();
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  randomSeed(analogRead(A3));

  for (int r = 0; r < GRID; r++)
    for (int c = 0; c < GRID; c++) {
      pinMode(MATRIX[r][c], OUTPUT);
      digitalWrite(MATRIX[r][c], LOW);
    }

  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  pinMode(LED_BTN_UP,    OUTPUT); digitalWrite(LED_BTN_UP,    LED_BTN_OFF);
  pinMode(LED_BTN_DOWN,  OUTPUT); digitalWrite(LED_BTN_DOWN,  LED_BTN_OFF);
  pinMode(LED_BTN_LEFT,  OUTPUT); digitalWrite(LED_BTN_LEFT,  LED_BTN_OFF);
  pinMode(LED_BTN_RIGHT, OUTPUT); digitalWrite(LED_BTN_RIGHT, LED_BTN_OFF);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  setColor(false, false, false);

  startGame();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  updateButtonFeedback();

  if (gameOver) return;   // brief guard during game-over animation

  // Blink food so it's visually distinct from the snake body
  if (millis() - lastFoodBlink >= FOOD_BLINK_MS) {
    lastFoodBlink = millis();
    foodVisible   = !foodVisible;
    drawMatrix();   // immediate redraw so blink is visible between moves
  }

  updateSpeedFromPot();
  readInput();

  if (millis() - lastMoveTime >= moveInterval) {
    lastMoveTime = millis();
    stepGame();
  }
}
