// ============================================================
//  Purple-Kit  —  DODGE ENEMY  v2  (Advanced Dodge)
//
//  HOW TO PLAY:
//    1. The single LED on the bottom row = your character
//    2. LEDs falling from the top = enemies
//    3. Turn the pot → move your character left / right
//    4. When an enemy reaches the bottom, DON'T be in its column!
//    5. Successfully dodge = score +1 ; get hit = lose a life
//    6. You have 3 lives. Enemies speed up as your score grows!
//
//  Control: POT (A1) — left / right movement
// ============================================================

#include <Arduino.h>

// ── MATRIX (row0=BOTTOM, row4=TOP) ───────────────────────────
const uint8_t MATRIX[5][5] = {
  {29,38,39,48,49},   // row0 = BOTTOM (player here)
  {30,37,40,47,50},
  {31,36,41,46,51},
  {32,35,42,45,52},
  {33,34,43,44,53}    // row4 = TOP (enemies fall from here)
};

// ── PINS ─────────────────────────────────────────────────────
const uint8_t RGB_R=9, RGB_G=10, RGB_B=11, BUZZER=3;
const uint8_t POT_PIN=A1;
const uint8_t BTN_LEFT=27, BTN_RIGHT=26;
const uint8_t LED_UP=21, LED_DOWN=18, LED_LEFT=20, LED_RIGHT=19;

// ── GAME CONSTANTS ───────────────────────────────────────────
const uint8_t  W                  = 5;
const uint8_t  H                  = 5;
const uint32_t TICK_START         = 1000;  // ms — starting tick interval (slow)
const uint32_t TICK_MIN           = 200;   // ms — fastest tick interval
const uint16_t TICK_REDUCE        = 10;    // ms reduction per point scored
const uint8_t  MAX_LIVES          = 3;
const uint8_t  THIRD_ENEMY_SCORE  = 10;   // score at which 3rd enemy appears

// ── GAME STATE ────────────────────────────────────────────────
int8_t   playerX   = 2;
int8_t   e1X = 0,  e1Y = 4;    // enemy 1
int8_t   e2X = 3,  e2Y = 8;    // enemy 2 (starts off-screen)
int8_t   e3X = 1,  e3Y = -1;   // enemy 3 (inactive until score threshold)
uint16_t score     = 0;
uint8_t  lives     = MAX_LIVES;
uint8_t  highScore = 0;
uint32_t lastTick  = 0;
bool     hitThisTick = false;   // prevents double-collision in one tick

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for(uint8_t r=0;r<H;r++) for(uint8_t c=0;c<W;c++) digitalWrite(MATRIX[r][c],LOW);
}
void ledOn(int8_t col, int8_t row) {
  if(col>=0 && col<W && row>=0 && row<H) digitalWrite(MATRIX[row][col],HIGH);
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
void hitSound()      { playNote(200,100); playNote(130,100); playNote(65,350); }
void dodgeSound()    { tone(BUZZER,1047,60); }   // quick beep: enemy dodged, point scored!
void gameOverSound() { playNote(440,200); playNote(349,200); playNote(196,500); }
void startSound()    { playNote(523,80); playNote(784,80); playNote(1047,180); }

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════

// Current tick interval in ms — decreases (speeds up) with score
uint32_t getTickMs() {
  int32_t ms = (int32_t)TICK_START - (int32_t)score * TICK_REDUCE;
  return (uint32_t)max((int32_t)TICK_MIN, ms);
}

// Returns true if a visible enemy is in the same column as the player
bool isDanger(int8_t eX, int8_t eY) {
  return (eY >= 0 && eY < H && eX == playerX);
}

// ═══════════════════════════════════════════════════════════
//  DRAW
// ═══════════════════════════════════════════════════════════
void drawFrame() {
  clearMatrix();
  ledOn(playerX, 0);                                // player (bottom row)
  if(e1Y >= 0 && e1Y < H) ledOn(e1X, e1Y);         // enemy 1
  if(e2Y >= 0 && e2Y < H) ledOn(e2X, e2Y);         // enemy 2
  if(e3Y >= 0 && e3Y < H) ledOn(e3X, e3Y);         // enemy 3 (if active)
}

// RGB warning: yellow = enemy in same column; red = 1 row away!
void updateWarningRGB() {
  bool sameCol  = isDanger(e1X,e1Y) || isDanger(e2X,e2Y) || isDanger(e3X,e3Y);
  bool critical = (e1Y==1&&e1X==playerX) || (e2Y==1&&e2X==playerX) || (e3Y==1&&e3X==playerX);

  if      (critical) setRGB(1,0,0);    // red: 1 step away — dodge NOW!
  else if (sameCol)  setRGB(1,1,0);   // yellow: danger column
  else               rgbOff();
}

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════
void explosionAnim() {
  setRGB(1,0,0);
  for(uint8_t i=0;i<4;i++){allOn();delay(100);clearMatrix();delay(100);}
  rgbOff();
}

// Show score as lit LEDs (1 LED = 1 point, fills matrix row by row)
void showScore(uint8_t s) {
  clearMatrix();
  uint8_t show = min(s, (uint8_t)25);
  for(uint8_t i=0;i<show;i++) ledOn(i%W, i/W);
  delay(3000);
  clearMatrix();
}

// 3-2-1 countdown bar on middle row
void countdown() {
  const uint16_t notes[3] = {659,784,1047};
  for(int8_t i=3;i>0;i--) {
    clearMatrix();
    uint8_t start = (W - (uint8_t)i) / 2;
    for(uint8_t c=0;c<(uint8_t)i;c++) ledOn(start+c, 2);
    if(i==3) setRGB(1,0,0); else if(i==2) setRGB(1,1,0); else setRGB(0,1,0);
    playNote(notes[3-i],150);
    delay(500);
    rgbOff();
  }
  clearMatrix();
  blinkAll(2,80);   // "GO!" flash
}

// ═══════════════════════════════════════════════════════════
//  COLLISION HANDLER
// ═══════════════════════════════════════════════════════════
void handleCollision() {
  if(hitThisTick) return;    // already processed a hit this tick
  hitThisTick = true;

  explosionAnim();
  hitSound();
  lives--;

  if(lives == 0) {
    // Game over
    gameOverSound();
    blinkAll(5, 120);
    showScore(score);          // show current run's score
    if(highScore > score) {    // then show all-time best if different
      delay(500);
      showScore(highScore);
    }
    delay(500);
    // Caller (loop) will invoke startGame() next
  } else {
    // Flash remaining lives
    for(uint8_t i=0;i<lives;i++){setRGB(1,0,0);delay(250);rgbOff();delay(200);}
    // Reset enemies — player gets a fresh start; score is preserved
    e1X = (int8_t)random(0,W);  e1Y = 4;
    e2X = (int8_t)random(0,W);  e2Y = 8;
    if(score >= THIRD_ENEMY_SCORE) { e3X = (int8_t)random(0,W); e3Y = 10; }
    else e3Y = -1;
    lastTick = millis();
  }
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  clearMatrix(); rgbOff();
  score   = 0;
  lives   = MAX_LIVES;
  playerX = 2;
  e1X = (int8_t)random(0,W);  e1Y = 4;
  e2X = (int8_t)random(0,W);  e2Y = 8;   // staggered: starts 4 ticks behind e1
  e3Y = -1;                               // inactive until score threshold
  hitThisTick = false;
  countdown();
  startSound();
  lastTick = millis();
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  for(uint8_t r=0;r<H;r++) for(uint8_t c=0;c<W;c++) pinMode(MATRIX[r][c],OUTPUT);
  pinMode(RGB_R,OUTPUT); pinMode(RGB_G,OUTPUT); pinMode(RGB_B,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(BTN_LEFT,INPUT_PULLUP);  pinMode(BTN_RIGHT,INPUT_PULLUP);
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
  hitThisTick  = false;

  // ── Player position: potentiometer ───────────────────────
  // map to (W+1) then constrain → equal zones across full pot range
  playerX = (int8_t)constrain(map(analogRead(POT_PIN), 0, 1023, 0, W), 0, W-1);

  // ── Mirror button LEDs (visual feedback only) ────────────
  digitalWrite(LED_LEFT,  digitalRead(BTN_LEFT)==LOW  ? LOW : HIGH);
  digitalWrite(LED_RIGHT, digitalRead(BTN_RIGHT)==LOW ? LOW : HIGH);

  // ── Enemy tick: move all active enemies one row down ─────
  if(now - lastTick >= getTickMs()) {
    lastTick = now;

    e1Y--; e2Y--;
    if(e3Y >= 0) e3Y--;

    // Enemy 1 passed bottom → SCORE + respawn at top
    if(e1Y < 0) {
      score++;
      if(score > highScore) highScore = score;
      dodgeSound();
      e1X = (int8_t)random(0,W);
      e1Y = 4;
    }
    // Enemy 2 passed bottom → respawn
    if(e2Y < 0) {
      e2X = (int8_t)random(0,W);
      e2Y = 4;
    }
    // Activate enemy 3 once score threshold is reached
    if(e3Y < 0 && score >= THIRD_ENEMY_SCORE) {
      e3X = (int8_t)random(0,W);
      e3Y = 6;   // appears 2 ticks later (off-screen buffer)
    }
  }

  // ── Collision check: enemy at row0 == player column? ─────
  if((e1Y==0 && e1X==playerX) ||
     (e2Y==0 && e2X==playerX) ||
     (e3Y==0 && e3X==playerX)) {
    handleCollision();
    if(lives == 0) {
      startGame();
      return;
    }
  }

  // ── RGB danger indicator ──────────────────────────────────
  if(!hitThisTick) updateWarningRGB();

  // ── Render ───────────────────────────────────────────────
  drawFrame();
  delay(15);
}
