// ============================================================
//  Purple-Kit  —  MORSE CODE TRANSLATOR  v3
//  row0=BOTTOM  row4=TOP  |  col0=LEFT  col4=RIGHT
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
const int BTN_DOT   = 27;   // White  — Dot  (.)
const int BTN_DASH  = 25;   // Green  — Dash (-)
const int BTN_SPACE = 26;   // Red    — Space / word separator
const int BTN_BACK  = 28;   // Blue   — Backspace (delete last symbol)

const int LED_BTN_DOT   = 20;   // White  LED
const int LED_BTN_DASH  = 18;   // Green  LED
const int LED_BTN_SPACE = 19;   // Red    LED
const int LED_BTN_BACK  = 21;   // Blue   LED

const int BUZZER = 3;
const int RGB_R = 9, RGB_G = 10, RGB_B = 11;

// Active-Low button LEDs
const int LED_BTN_ON  = LOW;
const int LED_BTN_OFF = HIGH;

// ── MORSE CODE DICTIONARY (A-Z) ──────────────────────────────
const char* morseCode[26] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....",
  "..",   ".---", "-.-",  ".-..", "--",   "-.",   "---",  ".--.",
  "--.-", ".-.",  "...",  "-",    "..-",  "...-", ".--",
  "-..-", "-.--", "--.."
};
const char alphabet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// ── 5×5 LETTER BITMAPS (row0=BOTTOM to row4=TOP) ─────────────
// Index 0-25 = A-Z. Index 26 = '?' (unknown sequence).
const byte letterBitmap[27][5] = {
  {B10001, B10001, B11111, B10001, B01110}, // A
  {B11110, B10001, B11110, B10001, B11110}, // B
  {B01111, B10000, B10000, B10000, B01111}, // C
  {B11110, B10001, B10001, B10001, B11110}, // D
  {B11111, B10000, B11110, B10000, B11111}, // E
  {B10000, B10000, B11110, B10000, B11111}, // F
  {B01111, B10001, B10111, B10000, B01111}, // G
  {B10001, B10001, B11111, B10001, B10001}, // H
  {B01110, B00100, B00100, B00100, B01110}, // I
  {B01100, B10010, B00010, B00010, B00111}, // J
  {B10001, B10010, B11100, B10010, B10001}, // K
  {B11111, B10000, B10000, B10000, B10000}, // L
  {B10001, B10001, B10101, B11011, B10001}, // M
  {B10001, B10011, B10101, B11001, B10001}, // N
  {B01110, B10001, B10001, B10001, B01110}, // O
  {B10000, B10000, B11110, B10001, B11110}, // P
  {B01101, B10010, B10101, B10001, B01110}, // Q
  {B10011, B10100, B11110, B10001, B11110}, // R
  {B11110, B00001, B01110, B10000, B01111}, // S
  {B00100, B00100, B00100, B00100, B11111}, // T
  {B01110, B10001, B10001, B10001, B10001}, // U
  {B00100, B01010, B10001, B10001, B10001}, // V
  {B10001, B11011, B10101, B10001, B10001}, // W
  {B10001, B01010, B00100, B01010, B10001}, // X
  {B00100, B00100, B00100, B01010, B10001}, // Y
  {B11111, B01000, B00100, B00010, B11111}, // Z
  {B00100, B00000, B00110, B10001, B01110}  // ? (index 26)
};

// ── TRANSLATOR STATE ─────────────────────────────────────────
String currentSequence  = "";
unsigned long lastPressTime = 0;
const unsigned long TIMEOUT = 1000;   // ms — auto-translate after 1 s of silence

bool prevDotState   = HIGH;
bool prevDashState  = HIGH;
bool prevSpaceState = HIGH;
bool prevBackState  = HIGH;
bool isMatrixShowing = false;

// ═══════════════════════════════════════════════════════════
//  RGB HELPER
// ═══════════════════════════════════════════════════════════
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}
void rgbFlash(bool r, bool g, bool b, int ms = 120) {
  setRGB(r, g, b); delay(ms); setRGB(0, 0, 0);
}

// ═══════════════════════════════════════════════════════════
//  MATRIX HELPERS
// ═══════════════════════════════════════════════════════════
void clearMatrix() {
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      digitalWrite(MATRIX[r][c], LOW);
  isMatrixShowing = false;
}

// Draw a letter by index (0-25 = A-Z, 26 = ?)
void drawLetter(int index) {
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++) {
      // bit 4 = leftmost column (col0), bit 0 = rightmost (col4)
      bool on = bitRead(letterBitmap[index][r], 4 - c);
      digitalWrite(MATRIX[r][c], on ? HIGH : LOW);
    }
  isMatrixShowing = true;
}

// ═══════════════════════════════════════════════════════════
//  TRANSLATE AND DISPLAY
// ═══════════════════════════════════════════════════════════
void translateAndDisplay() {
  int foundIndex = -1;

  for (int i = 0; i < 26; i++) {
    if (currentSequence == morseCode[i]) {
      Serial.print(alphabet[i]);
      foundIndex = i;
      break;
    }
  }

  if (foundIndex == -1) {
    Serial.print('?');
    foundIndex = 26;   // '?' bitmap
    rgbFlash(1, 0, 0); // red = unknown
  } else {
    rgbFlash(0, 1, 0); // green = success
  }

  drawLetter(foundIndex);
  currentSequence = "";
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);

  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++) {
      pinMode(MATRIX[r][c], OUTPUT);
      digitalWrite(MATRIX[r][c], LOW);
    }

  pinMode(BTN_DOT,   INPUT_PULLUP);
  pinMode(BTN_DASH,  INPUT_PULLUP);
  pinMode(BTN_SPACE, INPUT_PULLUP);
  pinMode(BTN_BACK,  INPUT_PULLUP);

  pinMode(LED_BTN_DOT,   OUTPUT); digitalWrite(LED_BTN_DOT,   LED_BTN_OFF);
  pinMode(LED_BTN_DASH,  OUTPUT); digitalWrite(LED_BTN_DASH,  LED_BTN_OFF);
  pinMode(LED_BTN_SPACE, OUTPUT); digitalWrite(LED_BTN_SPACE, LED_BTN_OFF);
  pinMode(LED_BTN_BACK,  OUTPUT); digitalWrite(LED_BTN_BACK,  LED_BTN_OFF);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  setRGB(0, 0, 0);

  Serial.println("===================================");
  Serial.println("  Morse Code Translator  Ready!   ");
  Serial.println("===================================");
  Serial.println(" White: Dot (.)   | Green: Dash (-)");
  Serial.println(" Red:   Space     | Blue:  Backspace");
  Serial.println(" Auto-translate after 1 s of silence");
  Serial.println("-----------------------------------");
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  bool currentDotState   = digitalRead(BTN_DOT);
  bool currentDashState  = digitalRead(BTN_DASH);
  bool currentSpaceState = digitalRead(BTN_SPACE);
  bool currentBackState  = digitalRead(BTN_BACK);

  // ── DOT button (White) ─────────────────────────────────
  if (currentDotState == LOW && prevDotState == HIGH) {
    if (isMatrixShowing) clearMatrix();
    currentSequence += ".";
    lastPressTime = millis();

    digitalWrite(LED_BTN_DOT, LED_BTN_ON);
    tone(BUZZER, 1200, 100);
    delay(100);
    digitalWrite(LED_BTN_DOT, LED_BTN_OFF);
  }

  // ── DASH button (Green) ────────────────────────────────
  if (currentDashState == LOW && prevDashState == HIGH) {
    if (isMatrixShowing) clearMatrix();
    currentSequence += "-";
    lastPressTime = millis();

    digitalWrite(LED_BTN_DASH, LED_BTN_ON);
    tone(BUZZER, 1200, 300);
    delay(300);
    digitalWrite(LED_BTN_DASH, LED_BTN_OFF);
  }

  // ── SPACE button (Red) — translate current sequence + word gap ──
  if (currentSpaceState == LOW && prevSpaceState == HIGH) {
    if (isMatrixShowing) clearMatrix();

    if (currentSequence.length() > 0) {
      translateAndDisplay();
    }

    Serial.print(' ');   // word separator in Serial Monitor
    lastPressTime = millis();

    digitalWrite(LED_BTN_SPACE, LED_BTN_ON);
    rgbFlash(1, 1, 0);   // yellow = space
    tone(BUZZER, 600, 150);
    delay(150);
    digitalWrite(LED_BTN_SPACE, LED_BTN_OFF);
  }

  // ── BACKSPACE button (Blue) — delete last symbol ───────
  if (currentBackState == LOW && prevBackState == HIGH) {
    if (isMatrixShowing) clearMatrix();

    if (currentSequence.length() > 0) {
      currentSequence.remove(currentSequence.length() - 1);
      lastPressTime = millis();

      digitalWrite(LED_BTN_BACK, LED_BTN_ON);
      rgbFlash(0, 0, 1);   // blue = backspace
      tone(BUZZER, 2000, 50); delay(50);
      tone(BUZZER, 1500, 50); delay(50);
      digitalWrite(LED_BTN_BACK, LED_BTN_OFF);

      Serial.print('[');
      Serial.print(currentSequence);   // show remaining sequence after delete
      Serial.print(']');
    }
  }

  // Update previous button states
  prevDotState   = currentDotState;
  prevDashState  = currentDashState;
  prevSpaceState = currentSpaceState;
  prevBackState  = currentBackState;

  // ── Auto-translate after TIMEOUT ms of inactivity ──────
  if (currentSequence.length() > 0 && (millis() - lastPressTime > TIMEOUT)) {
    translateAndDisplay();
  }
}
