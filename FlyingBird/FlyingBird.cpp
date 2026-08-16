// ============================================================
//  Purple-Kit  —  FLYING BIRD  v2  (Elementary-friendly)
//
//  HOW TO PLAY:
//    1. The BLINKING LED on the bottom row = your bird
//    2. A wall descends from the top, row by row
//    3. The wall has ONE dark gap = the safe passage
//    4. Turn the pot: align your bird under the gap
//    5. When the wall reaches the bottom:
//         bird in the gap  → score point!
//         bird NOT in gap  → lose a life (3 lives total)
//
//  Control: POT (A1) only
// ============================================================

#include <Arduino.h>

// ── MATRIX (row0=BOTTOM, row4=TOP) ───────────────────────────
const uint8_t MATRIX[5][5] = {
  {29,38,39,48,49},   // row0 = BOTTOM (bird here)
  {30,37,40,47,50},
  {31,36,41,46,51},
  {32,35,42,45,52},
  {33,34,43,44,53}    // row4 = TOP (wall starts here)
};

// ── PINS ─────────────────────────────────────────────────────
const uint8_t RGB_R=9, RGB_G=10, RGB_B=11, BUZZER=3;
const uint8_t POT_PIN=A1;
const uint8_t BTN_LEFT=27, BTN_RIGHT=26;
const uint8_t LED_UP=21, LED_DOWN=18, LED_LEFT=20, LED_RIGHT=19;

// ── GAME CONSTANTS ───────────────────────────────────────────
const uint8_t  W            = 5;
const uint8_t  H            = 5;
const uint32_t WALL_MS      = 6000;  // ms — starting step interval (slow)
const uint32_t WALL_MS_MIN  = 2000;  // ms — fastest step interval
const uint16_t SPEED_REDUCE = 8;     // ms reduction per point scored
const uint8_t  MAX_LIVES    = 3;
const uint32_t BLINK_MS     = 500;   // ms — bird blink period

// ── GAME STATE ───────────────────────────────────────────────
int8_t   birdX       = 2;
int8_t   wallY       = 4;
uint8_t  gapCol      = 2;
uint16_t score       = 0;
uint8_t  lives       = MAX_LIVES;
uint8_t  highScore   = 0;
bool     birdVisible = true;
bool     firstGame   = true;   // show tutorial only on the very first game

uint32_t lastWallStep = 0;
uint32_t lastBlink    = 0;

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for(uint8_t r=0;r<H;r++) for(uint8_t c=0;c<W;c++) digitalWrite(MATRIX[r][c],LOW);
}
void ledOn(int8_t col, int8_t row) {
  if(col>=0&&col<W&&row>=0&&row<H) digitalWrite(MATRIX[row][col],HIGH);
}
void allOn() {
  for(uint8_t r=0;r<H;r++) for(uint8_t c=0;c<W;c++) digitalWrite(MATRIX[r][c],HIGH);
}
void blinkAll(uint8_t n, uint16_t ms) {
  for(uint8_t i=0;i<n;i++){allOn();delay(ms);clearMatrix();delay(ms);}
}

// ═══════════════════════════════════════════════════════════
//  RGB
// ═══════════════════════════════════════════════════════════
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R,r?HIGH:LOW);
  digitalWrite(RGB_G,g?HIGH:LOW);
  digitalWrite(RGB_B,b?HIGH:LOW);
}
void rgbOff() { setRGB(0,0,0); }

// ═══════════════════════════════════════════════════════════
//  SOUND
// ═══════════════════════════════════════════════════════════
void playNote(uint16_t hz, uint16_t ms) { tone(BUZZER,hz,ms); delay(ms+10); }
void passSound()     { playNote(784,80); playNote(1047,150); }
void hitSound()      { playNote(200,100); playNote(130,100); playNote(65,300); }
void gameOverSound() { playNote(440,200); playNote(349,200); playNote(196,500); }
void startSound()    { playNote(523,100); playNote(784,100); playNote(1047,200); }

// ═══════════════════════════════════════════════════════════
//  WALL SPEED
// ═══════════════════════════════════════════════════════════
uint32_t wallStepMs() {
  int32_t ms = (int32_t)WALL_MS - (int32_t)score * SPEED_REDUCE;
  return (uint32_t)max((int32_t)WALL_MS_MIN, ms);
}

// ═══════════════════════════════════════════════════════════
//  DRAW FRAME
// ═══════════════════════════════════════════════════════════
void drawFrame() {
  clearMatrix();
  // Wall: full horizontal line with a single gap
  if(wallY >= 0 && wallY < H) {
    for(uint8_t c=0;c<W;c++) if(c != (uint8_t)gapCol) ledOn(c, wallY);
  }
  // Bird: bottom row, blinks so player can spot it easily
  if(birdVisible) ledOn(birdX, 0);
}

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════
void explosionAnim() {
  setRGB(1,0,0);
  blinkAll(4, 100);
  rgbOff();
}

// Show score as lit LEDs (max 25), fills left-to-right bottom-to-top
void showScore(uint8_t s) {
  clearMatrix();
  uint8_t show = min(s, (uint8_t)25);
  for(uint8_t i=0;i<show;i++) ledOn(i%W, i/W);
  delay(3000);
  clearMatrix();
}

// 3-2-1 countdown bar on middle row
void countdown() {
  const uint16_t notes[3]={659,784,1047};
  for(int8_t i=3;i>0;i--) {
    clearMatrix();
    uint8_t start = (W-(uint8_t)i)/2;
    for(uint8_t c=0;c<(uint8_t)i;c++) ledOn(start+c, 2);
    if(i==3) setRGB(1,0,0); else if(i==2) setRGB(1,1,0); else setRGB(0,1,0);
    playNote(notes[3-i], 150); delay(500); rgbOff();
  }
  clearMatrix(); blinkAll(2, 80);
}

// ── Tutorial: step-by-step demo of game mechanics ────────────
void showTutorial() {
  // STEP 1: "This is you" — bird blinks at bottom, pot controls it
  setRGB(0,0,1);   // blue = bird colour
  for(uint8_t i=0; i<8; i++) {
    clearMatrix();
    int8_t gX = (int8_t)constrain(map(analogRead(POT_PIN), 0, 1023, 0, W), 0, W-1);
    ledOn(gX, 0);
    delay(350);
    clearMatrix(); delay(200);
  }
  rgbOff(); delay(500);

  // STEP 2: "Watch the gap" — wall at row3, gap at col2, bird aligned → green = safe
  clearMatrix();
  ledOn(2, 0);
  for(uint8_t c=0;c<W;c++) if(c!=2) ledOn(c, 3);
  setRGB(0,1,0);   // green = safe!
  delay(3000);
  rgbOff();

  // STEP 3: Wall descends step by step — player sees how it works
  for(int8_t y=3; y>=0; y--) {
    clearMatrix();
    ledOn(2, 0);
    for(uint8_t c=0;c<W;c++) if(c!=2) ledOn(c, y);
    if(y==1) setRGB(1,1,0);   // yellow: wall is close!
    else     setRGB(0,1,0);   // green: still aligned
    delay(800);
  }
  // Wall hit row0, bird was in gap → passed!
  rgbOff();
  setRGB(0,1,0); playNote(1047,200); rgbOff();
  delay(500);

  // STEP 4: "What if you're in the wrong spot" — bird at col0, gap at col2 → danger
  clearMatrix();
  ledOn(0, 0);
  for(uint8_t c=0;c<W;c++) if(c!=2) ledOn(c, 1);
  setRGB(1,1,0);   // yellow = danger!
  delay(2000);

  // Collision demo
  setRGB(1,0,0);
  hitSound();
  blinkAll(3, 120);
  rgbOff(); delay(800);
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  clearMatrix(); rgbOff();
  score       = 0;
  lives       = MAX_LIVES;
  birdX       = 2;
  birdVisible = true;
  wallY       = 4;
  gapCol      = (uint8_t)random(0, W);

  if(firstGame) {
    firstGame = false;
    showTutorial();
  }

  countdown();
  startSound();
  lastWallStep = lastBlink = millis();
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  for(uint8_t r=0;r<H;r++) for(uint8_t c=0;c<W;c++) pinMode(MATRIX[r][c],OUTPUT);
  pinMode(RGB_R,OUTPUT); pinMode(RGB_G,OUTPUT); pinMode(RGB_B,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(BTN_LEFT,INPUT_PULLUP); pinMode(BTN_RIGHT,INPUT_PULLUP);
  pinMode(LED_UP,OUTPUT);    digitalWrite(LED_UP,HIGH);
  pinMode(LED_DOWN,OUTPUT);  digitalWrite(LED_DOWN,HIGH);
  pinMode(LED_LEFT,OUTPUT);  digitalWrite(LED_LEFT,HIGH);
  pinMode(LED_RIGHT,OUTPUT); digitalWrite(LED_RIGHT,HIGH);
  randomSeed(analogRead(A0));
  startGame();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  uint32_t now = millis();

  // ── Bird X: potentiometer only ───────────────────────────
  // map to (W+1) then constrain → equal zones for all 5 columns
  birdX = (int8_t)constrain(map(analogRead(POT_PIN), 0, 1023, 0, W), 0, W-1);

  // ── Bird blink ───────────────────────────────────────────
  if(now - lastBlink >= BLINK_MS) {
    lastBlink    = now;
    birdVisible  = !birdVisible;
  }

  // ── Wall step ────────────────────────────────────────────
  if(now - lastWallStep >= wallStepMs()) {
    lastWallStep = now;
    wallY--;

    // Wall reached row0 — moment of truth!
    if(wallY == 0) {
      if(birdX == (int8_t)gapCol) {
        // ── PASSED: bird is in the gap ──
        score++;
        if(score > highScore) highScore = score;
        setRGB(0,1,0);
        passSound();
        delay(150); rgbOff();
        // Spawn next wall
        wallY       = 4;
        gapCol      = (uint8_t)random(0, W);
        lastWallStep = millis();

      } else {
        // ── HIT: bird is not in the gap ──
        explosionAnim();
        hitSound();
        lives--;

        if(lives == 0) {
          // GAME OVER
          gameOverSound();
          blinkAll(5, 130);
          if(highScore > 0) showScore(highScore);
          delay(500);
          startGame();
          return;
        }

        // Show remaining lives then reset wall
        for(uint8_t i=0;i<lives;i++){setRGB(1,0,0);delay(250);rgbOff();delay(200);}
        wallY        = 4;
        gapCol       = (uint8_t)random(0, W);
        lastWallStep  = millis();
      }
    }
  }

  // ── RGB alignment hint (last 2 rows before impact) ───────
  if(wallY == 1) {
    // One row away — critical!
    if(birdX == (int8_t)gapCol) setRGB(0,1,0);   // green: aligned!
    else                         setRGB(1,0,0);   // red:   move now!
  } else if(wallY == 2) {
    if(birdX == (int8_t)gapCol) setRGB(0,1,0);   // green: good position
    else                         setRGB(1,1,0);   // yellow: watch out
  } else {
    rgbOff();
  }

  // ── Render ───────────────────────────────────────────────
  drawFrame();
  delay(15);
}
