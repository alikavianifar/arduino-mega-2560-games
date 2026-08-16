// ============================================================
//  Purple-Kit  —  LASER CORRIDOR  (Secret Agent)
//
//  HOW TO PLAY:
//    Navigate your agent through 5 laser floors from bottom
//    to top. On each floor the RGB gives a colour hint:
//      BLUE  = you are in the 2-column "safe zone" (50/50)
//      RED   = you are definitely hitting a laser
//    Press the button to commit. Land on the EXACT safe column
//    to advance. Wrong column = back to floor 0!
//
//  Control: POT (A1) — move agent left/right
//           BTN (25) — commit position / restart after win
// ============================================================

#include <Arduino.h>

// ── PINS ─────────────────────────────────────────────────────
const int POT_PIN  = A1;
const int BTN      = 25;   // DOWN button — submit position
const int BUZZER   = 3;
const int RGB_R    = 9;
const int RGB_G    = 10;
const int RGB_B    = 11;

// ── MATRIX PIN MAP ───────────────────────────────────────────
const int MATRIX[5][5] = {
  {29, 38, 39, 48, 49},   // row0 = BOTTOM (starting floor)
  {30, 37, 40, 47, 50},
  {31, 36, 41, 46, 51},
  {32, 35, 42, 45, 52},
  {33, 34, 43, 44, 53}    // row4 = TOP    (goal)
};

// ── GAME STATE ───────────────────────────────────────────────
int  currentFloor = 0;
int  agentX       = 2;
bool gameWon      = false;

int safeCol[5];    // the ONE truly safe column per floor (0-4)
int hintCol[5];    // the decoy column shown alongside safeCol in the blue zone

bool prevBtn = HIGH;

// ═══════════════════════════════════════════════════════════
//  GENERATE LASER LAYOUT
//  For each floor: pick a random safe column, then pick one
//  neighbour as the "decoy" blue column. The player sees
//  exactly 2 blue columns and must guess which is real (50/50).
// ═══════════════════════════════════════════════════════════
void generateLasers() {
  for (int i = 0; i < 5; i++) {
    safeCol[i] = random(0, 5);

    bool canRight = (safeCol[i] < 4);
    bool canLeft  = (safeCol[i] > 0);

    if (canRight && canLeft) {
      // Both neighbours exist — randomly pick left or right
      hintCol[i] = (random(0, 2) == 0) ? safeCol[i] - 1 : safeCol[i] + 1;
    } else if (canRight) {
      hintCol[i] = safeCol[i] + 1;
    } else {
      hintCol[i] = safeCol[i] - 1;
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      digitalWrite(MATRIX[r][c], LOW);
}

void fillMatrix() {
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      digitalWrite(MATRIX[r][c], HIGH);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}

// ═══════════════════════════════════════════════════════════
//  RGB HINT
//  Blue  = agent is in the 2-column safe zone (50/50 guess)
//  Red   = agent is definitely on a laser (certain death)
// ═══════════════════════════════════════════════════════════
void showHint() {
  bool inBlueZone = (agentX == safeCol[currentFloor] ||
                     agentX == hintCol[currentFloor]);
  if (inBlueZone) setRGB(0, 0, 1);   // blue  = might be safe
  else            setRGB(1, 0, 0);   // red   = definitely laser
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  pinMode(POT_PIN, INPUT);
  pinMode(BTN,     INPUT_PULLUP);
  pinMode(BUZZER,  OUTPUT);
  pinMode(RGB_R,   OUTPUT);
  pinMode(RGB_G,   OUTPUT);
  pinMode(RGB_B,   OUTPUT);

  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++) {
      pinMode(MATRIX[r][c], OUTPUT);
      digitalWrite(MATRIX[r][c], LOW);
    }

  randomSeed(analogRead(A0));
  generateLasers();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {

  // ── WIN STATE: hold full matrix + green RGB, wait for restart ──
  if (gameWon) {
    fillMatrix();
    setRGB(0, 1, 0);   // solid green = "you won!"

    bool curBtn = digitalRead(BTN);
    if (curBtn == LOW && prevBtn == HIGH) {
      clearMatrix();
      setRGB(0, 0, 0);
      currentFloor = 0;
      gameWon      = false;
      generateLasers();
    }
    prevBtn = curBtn;
    return;
  }

  // ── 1. Read agent position from potentiometer ─────────────
  // BUG FIX: map to (W+1) then constrain → equal zones for all 5 columns
  agentX = constrain(map(analogRead(POT_PIN), 0, 1023, 0, 5), 0, 4);

  // ── 2. Draw current game state ────────────────────────────
  clearMatrix();
  // Floors already cleared are fully lit (show progress)
  for (int r = 0; r < currentFloor; r++)
    for (int c = 0; c < 5; c++)
      digitalWrite(MATRIX[r][c], HIGH);
  // Agent position on current floor
  digitalWrite(MATRIX[currentFloor][agentX], HIGH);

  // ── 3. RGB hint ───────────────────────────────────────────
  showHint();

  // ── 4. Button — commit position ──────────────────────────
  bool curBtn    = digitalRead(BTN);
  bool btnPressed = (curBtn == LOW && prevBtn == HIGH);
  prevBtn = curBtn;

  if (btnPressed) {
    if (agentX == safeCol[currentFloor]) {
      // ── SUCCESS: correct safe column ──
      setRGB(0, 1, 0);
      tone(BUZZER, 1500, 100);
      delay(150);

      currentFloor++;

      if (currentFloor > 4) {
        // ── ALL 5 FLOORS CLEARED — WIN ──
        gameWon = true;
        tone(BUZZER, 2200, 500);
        // Victory blink animation
        for (int f = 0; f < 3; f++) {
          fillMatrix(); delay(100);
          clearMatrix(); delay(100);
        }
        // (win display is maintained at the top of the next loop iteration)
      }

    } else {
      // ── HIT A LASER (wrong column, even if blue zone decoy) ──
      setRGB(1, 0, 0);
      tone(BUZZER, 200, 400);
      delay(400);
      currentFloor = 0;
      generateLasers();   // new laser layout every time you restart
    }
  }

  delay(20);
}
