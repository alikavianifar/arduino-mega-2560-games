// ============================================================
//  Purple-Kit  —  SIMON SAYS  v2
//
//  HOW TO PLAY:
//    1. Watch the buttons light up in sequence → MEMORISE IT
//    2. All buttons blink 2× + blue RGB → YOUR TURN!
//    3. Press the buttons in the same order
//    4. Correct press: button flashes + tone plays
//    5. Wrong press: red RGB + all buttons blink
//    6. You have 2 lives. The sequence grows and speeds up!
//
//  Button → Tone mapping:
//    BTN 25 (DOWN)  → LED 18 → C4  (262 Hz)
//    BTN 26 (RIGHT) → LED 19 → E4  (330 Hz)
//    BTN 27 (LEFT)  → LED 20 → G4  (392 Hz)
//    BTN 28 (UP)    → LED 21 → C5  (523 Hz)
// ============================================================

#include <Arduino.h>

// ── BUTTON LEDs (Active-LOW, index 1-4) ──────────────────────
const uint8_t BLED[5] = {0, 18, 19, 20, 21};

// ── BUTTONS (INPUT_PULLUP, LOW = pressed, index 1-4) ─────────
const uint8_t BTN[5] = {0, 25, 26, 27, 28};

// ── BUTTON TONES in Hz (index 1-4) ───────────────────────────
const uint16_t BTN_NOTE[5] = {0, 262, 330, 392, 523};

// ── RGB & BUZZER ─────────────────────────────────────────────
const uint8_t RGB_R=9, RGB_G=10, RGB_B=11, BUZZER=3;

// ── GAME CONSTANTS ───────────────────────────────────────────
const uint8_t  MAX_LEN   = 20;   // maximum sequence length (win condition)
const uint16_t MS_START  = 600;  // ms — starting note duration
const uint16_t MS_MIN    = 150;  // ms — fastest note duration
const uint16_t MS_REDUCE = 20;   // ms reduction per round
const uint16_t MS_GAP    = 120;  // ms silence between notes
const uint8_t  MAX_LIVES = 2;

// ── GAME STATE ───────────────────────────────────────────────
uint8_t sequence[MAX_LEN + 1];  // 1-indexed; values are 1-4
uint8_t seqLen    = 0;
uint8_t inputStep = 0;          // how far into the sequence the player has confirmed
uint8_t lives     = MAX_LIVES;
uint8_t bestScore = 0;          // longest sequence completed this session

uint8_t prev[5];   // last button state for edge detection (index 1-4)

// Game phase — blocks input while Simon is showing the sequence
enum Phase { SHOWING, WAITING };
Phase phase = SHOWING;

// ═══════════════════════════════════════════════════════════
//  RGB
// ═══════════════════════════════════════════════════════════
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r ? HIGH : LOW);
  digitalWrite(RGB_G, g ? HIGH : LOW);
  digitalWrite(RGB_B, b ? HIGH : LOW);
}
void rgbOff() { setRGB(0, 0, 0); }

// ═══════════════════════════════════════════════════════════
//  SOUND
// ═══════════════════════════════════════════════════════════
void playNote(uint16_t hz, uint16_t ms) { tone(BUZZER, hz, ms); delay(ms + 10); }

void startSound()     { playNote(523,100); playNote(659,100); playNote(784,100); playNote(1047,200); }
void successSound()   { playNote(1047,80); playNote(1319,80); playNote(1568,200); }
void errorSound()     { playNote(200,120); playNote(130,120); playNote(65,400); }
void gameOverSound()  { playNote(440,200); playNote(349,200); playNote(196,500); }
void milestoneSound() { playNote(523,55); playNote(659,55); playNote(784,55);
                        playNote(880,55); playNote(1047,55); playNote(1319,200); }

// ═══════════════════════════════════════════════════════════
//  BUTTON LED HELPERS
// ═══════════════════════════════════════════════════════════
void allBLED(uint8_t val) {
  for (uint8_t i=1; i<=4; i++) digitalWrite(BLED[i], val);
}

// Flash one button LED + play its tone, then pause (used in both showing and feedback)
void showButton(uint8_t idx, uint16_t ms) {
  digitalWrite(BLED[idx], LOW);         // LED ON (active-low)
  tone(BUZZER, BTN_NOTE[idx], ms);
  delay(ms);
  digitalWrite(BLED[idx], HIGH);        // LED OFF
  delay(MS_GAP);
}

// ═══════════════════════════════════════════════════════════
//  "YOUR TURN" TRANSITION SIGNAL
//  Called after the sequence finishes showing.
//  All button LEDs blink 2× → blue RGB stays on = "enter input now"
// ═══════════════════════════════════════════════════════════
void yourTurnSignal() {
  for (uint8_t i=0; i<2; i++) {
    allBLED(LOW);  delay(120);
    allBLED(HIGH); delay(100);
  }
  setRGB(0, 0, 1);   // solid blue = "it's your turn"
}

// ═══════════════════════════════════════════════════════════
//  PLAY SEQUENCE (Simon's turn — blocks all button input)
// ═══════════════════════════════════════════════════════════
void playSequence() {
  phase = SHOWING;

  // Note duration decreases with sequence length (gets harder)
  uint16_t ms = max((int16_t)MS_MIN,
                    (int16_t)MS_START - (int16_t)seqLen * MS_REDUCE);

  allBLED(HIGH);   // all LEDs off before showing
  rgbOff();
  delay(400);

  setRGB(0, 1, 0);   // green RGB = "Simon is showing"
  for (uint8_t i=1; i<=seqLen; i++) {
    showButton(sequence[i], ms);
  }
  rgbOff();
  delay(200);

  // Transition: "YOUR TURN!"
  yourTurnSignal();

  // Switch to waiting phase — loop() will now accept input
  phase     = WAITING;
  inputStep = 0;

  // Flush stale button states so an accidental hold during the
  // showing phase doesn't register as an immediate press
  for (uint8_t i=1; i<=4; i++) prev[i] = digitalRead(BTN[i]);
}

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════
void countdown() {
  const uint16_t notes[3] = {659, 784, 1047};
  for (int8_t i=3; i>0; i--) {
    allBLED(HIGH); rgbOff();
    // Light i button LEDs as the countdown number
    for (uint8_t b=1; b<=(uint8_t)i; b++) digitalWrite(BLED[b], LOW);
    if      (i==3) setRGB(1,0,0);
    else if (i==2) setRGB(1,1,0);
    else           setRGB(0,1,0);
    playNote(notes[3-i], 150);
    delay(500);
    allBLED(HIGH); rgbOff();
  }
  // "Go!" — buttons ripple left to right
  for (uint8_t b=1; b<=4; b++) {
    digitalWrite(BLED[b], LOW);  delay(80);
    digitalWrite(BLED[b], HIGH); delay(40);
  }
}

void celebrateAnim() {
  for (uint8_t i=0; i<3; i++) {
    allBLED(LOW);  delay(100);
    allBLED(HIGH); delay(80);
  }
}

// ═══════════════════════════════════════════════════════════
//  START / RESET GAME
// ═══════════════════════════════════════════════════════════
void startGame() {
  allBLED(HIGH); rgbOff();
  seqLen    = 0;
  lives     = MAX_LIVES;
  inputStep = 0;
  for (uint8_t i=1; i<=4; i++) prev[i] = HIGH;

  countdown();
  startSound();
  delay(400);

  // Add first element and show it
  sequence[1] = (uint8_t)random(1, 5);
  seqLen      = 1;
  playSequence();   // show → "your turn" signal → phase = WAITING
}

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  pinMode(RGB_R, OUTPUT); pinMode(RGB_G, OUTPUT); pinMode(RGB_B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  for (uint8_t i=1; i<=4; i++) {
    pinMode(BLED[i], OUTPUT); digitalWrite(BLED[i], HIGH);   // all off
    pinMode(BTN[i],  INPUT_PULLUP);
  }
  randomSeed(analogRead(A0));
  startGame();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP — only reads buttons during WAITING phase
// ═══════════════════════════════════════════════════════════
void loop() {
  if (phase != WAITING) return;   // block all input while Simon is showing

  for (uint8_t btn=1; btn<=4; btn++) {
    uint8_t cur = digitalRead(BTN[btn]);

    if (cur == LOW && prev[btn] == HIGH) {
      // ── Button just pressed — give immediate feedback ──
      showButton(btn, 280);

      if (btn == sequence[inputStep + 1]) {
        // ── CORRECT ──
        inputStep++;

        if (inputStep == seqLen) {
          // ── ROUND COMPLETE ──
          if (seqLen > bestScore) bestScore = seqLen;

          if (seqLen % 5 == 0) {
            // Milestone every 5 rounds
            setRGB(0,1,1); milestoneSound(); celebrateAnim(); rgbOff();
          } else {
            successSound(); setRGB(0,1,0); delay(250); rgbOff();
          }
          delay(300);

          if (seqLen < MAX_LEN) {
            // Add one more element and replay
            sequence[seqLen + 1] = (uint8_t)random(1, 5);
            seqLen++;
            playSequence();
          } else {
            // Reached maximum length — player wins!
            setRGB(1,1,0);
            milestoneSound(); milestoneSound();
            celebrateAnim(); celebrateAnim();
            rgbOff();
            startGame();
            return;
          }
        }

        // Keep blue RGB on between correct presses
        setRGB(0, 0, 1);

      } else {
        // ── WRONG ──
        rgbOff();
        setRGB(1, 0, 0);
        errorSound();
        // Triple-blink all button LEDs
        for (uint8_t f=0; f<3; f++) {
          allBLED(LOW); delay(150); allBLED(HIGH); delay(80);
        }
        rgbOff();

        lives--;
        inputStep = 0;

        if (lives == 0) {
          // ── GAME OVER ──
          gameOverSound();
          // Show best score using button LEDs (max 4)
          if (bestScore > 0) {
            allBLED(HIGH);
            uint8_t show = min(bestScore, (uint8_t)4);
            for (uint8_t b=1; b<=show; b++) digitalWrite(BLED[b], LOW);
            delay(3000);
            allBLED(HIGH);
          }
          delay(500);
          startGame();
          return;
        } else {
          // 1 life remaining — show remaining lives, then replay same sequence
          for (uint8_t i=0; i<lives; i++) {
            setRGB(1,0,0); delay(250); rgbOff(); delay(200);
          }
          delay(500);
          playSequence();   // same sequence again — no new element added
        }
      }
    }

    prev[btn] = cur;
  }

  delay(8);
}
