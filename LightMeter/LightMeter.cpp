// ============================================================
//  Purple-Kit  —  LIGHT METER
//  Reads ambient light via LDR and displays a bar graph
//  on the 5×5 LED matrix. Auto-calibrates to the range of
//  light levels seen since power-on.
//
//  row0=BOTTOM  row4=TOP  |  col0=LEFT  col4=RIGHT
// ============================================================

#include <Arduino.h>

// ── PINS ─────────────────────────────────────────────────────
const int LDR_PIN  = A0;   // LDR (Light Dependent Resistor)
const int RGB_R    = 9;
const int RGB_G    = 10;
const int RGB_B    = 11;

// ── MATRIX PIN MAP ───────────────────────────────────────────
const int MATRIX[5][5] = {
  {29, 38, 39, 48, 49},   // row0 = BOTTOM
  {30, 37, 40, 47, 50},   // row1
  {31, 36, 41, 46, 51},   // row2 = MIDDLE
  {32, 35, 42, 45, 52},   // row3
  {33, 34, 43, 44, 53}    // row4 = TOP
};

// ── AUTO-CALIBRATION ─────────────────────────────────────────
// Tracks the lowest and highest LDR readings seen so far.
// The bar graph always fills the full 5 rows across that range.
int ldrMin = 1023;
int ldrMax = 0;

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      digitalWrite(MATRIX[r][c], LOW);
}

// Light all 5 LEDs in a single row
void lightRow(int row) {
  for (int c = 0; c < 5; c++)
    digitalWrite(MATRIX[row][c], HIGH);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);

  pinMode(LDR_PIN, INPUT);
  pinMode(RGB_R,   OUTPUT);
  pinMode(RGB_G,   OUTPUT);
  pinMode(RGB_B,   OUTPUT);

  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++) {
      pinMode(MATRIX[r][c], OUTPUT);
      digitalWrite(MATRIX[r][c], LOW);
    }

  Serial.println("=================================");
  Serial.println("  Light Meter  —  Purple-Kit     ");
  Serial.println("=================================");
  Serial.println("Wave your hand or use a torch to");
  Serial.println("see the bar graph update.        ");
  Serial.println("---------------------------------");
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  int ldrValue = analogRead(LDR_PIN);

  // ── Auto-calibrate running min/max ────────────────────
  if (ldrValue < ldrMin) ldrMin = ldrValue;
  if (ldrValue > ldrMax) ldrMax = ldrValue;

  // Enforce a minimum useful range to avoid division issues
  // when the scene is very uniform (e.g. just after power-on).
  int range = ldrMax - ldrMin;
  if (range < 10) range = 10;

  // ── Map raw reading → bar level 0-5 ───────────────────
  // level 0 = very dark, level 5 = brightest seen so far
  int level = map(ldrValue, ldrMin, ldrMin + range, 0, 5);
  level = constrain(level, 0, 5);

  // ── Draw bar graph (rows fill from bottom up) ─────────
  clearMatrix();
  for (int row = 0; row < level; row++)
    lightRow(row);

  // ── RGB color reflects light level ───────────────────
  //   Dark  (0)   → White  (all LEDs on = max visibility in the dark)
  //   Dim   (1-2) → Blue
  //   Medium(3-4) → Green
  //   Bright(5)   → Red
  if      (level == 0)            setRGB(1, 1, 1);   // white — very dark
  else if (level <= 2)            setRGB(0, 0, 1);   // blue  — dim
  else if (level <= 4)            setRGB(0, 1, 0);   // green — moderate
  else                            setRGB(1, 0, 0);   // red   — bright

  // ── Serial output for classroom / debugging ───────────
  Serial.print("LDR: ");
  Serial.print(ldrValue);
  Serial.print("  [min=");
  Serial.print(ldrMin);
  Serial.print(" max=");
  Serial.print(ldrMax);
  Serial.print("]  Level: ");
  Serial.println(level);

  delay(100);   // ~10 updates per second — smooth but not flooding Serial
}
