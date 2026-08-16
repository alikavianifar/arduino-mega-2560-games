# Simon Says

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Memory / Rhythm  
**Players:** 1

---

## How to Play

Simon shows you a sequence by flashing button LEDs and playing tones. Watch carefully, then repeat it in the same order. Each round the sequence grows by one. How long can you remember?

### Step by step

1. **Watch:** Simon lights up the buttons one by one (green RGB = Simon's turn). Memorise the order.
2. **Your Turn Signal:** All four button LEDs blink twice → RGB turns solid **blue** → now it's your turn!
3. **Repeat:** Press the buttons in the same order. Each correct press flashes the LED and plays the tone.
4. **Round complete:** If you finish the sequence, Simon adds one more and shows it again.
5. **Wrong press:** Red RGB + triple blink. Lose a life. The same sequence is shown again from the start.
6. **Game Over:** Lose both lives → game resets. Your best score is shown using the button LEDs.

---

## Controls

| Button | Pin | LED | Tone |
|---|---|---|---|
| DOWN (green) | 25 | 18 | C4 — 262 Hz |
| RIGHT (red) | 26 | 19 | E4 — 330 Hz |
| LEFT (white) | 27 | 20 | G4 — 392 Hz |
| UP (blue) | 28 | 21 | C5 — 523 Hz |

No potentiometer or LDR used.

---

## Rules

- You have **2 lives**.
- **Correct sequence** → score +1 round, sequence grows by 1.
- **Wrong button** → lose 1 life. The same sequence replays from the beginning (no new element added).
- **0 lives** → Game Over. High score shown, auto-restart.
- Reach **20 rounds** → you win! Special celebration, then auto-restart.

---

## Phase Indicator (RGB)

| RGB Color | Meaning |
|---|---|
| 🟢 Green | Simon is showing the sequence — **watch, don't press** |
| 🔵 Blue (solid) | Your turn — press the sequence now |
| 🔴 Red | Wrong button pressed |
| 🩵 Cyan | Milestone reached (every 5 rounds) |
| 🟡 Yellow | Maximum reached — you won! |

---

## Speed

The note display duration decreases by **20 ms per round**, starting at 600 ms and bottoming out at 150 ms. By round 20 the sequence flashes very fast!

| Round | Note duration |
|---|---|
| 1 | 600 ms |
| 5 | 500 ms |
| 10 | 400 ms |
| 15 | 300 ms |
| 20 (win) | 200 ms |

---

## Milestones

Every **5 rounds** (rounds 5, 10, 15, 20) a special fanfare plays and the button LEDs celebrate. Reaching round 20 is the ultimate win.

---

## Score Display at Game Over

The **button LEDs show your best score**: 1 LED lit = 1 round completed (max 4 shown). So if you completed 3 rounds before losing, 3 button LEDs light up for 3 seconds.

---

## Countdown Animation

At the start of each game a 3-2-1 countdown plays using the button LEDs (3 lit → 2 → 1) with matching RGB colours, then a left-to-right ripple signals "Go!"

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| DOWN button | 25 — INPUT_PULLUP |
| RIGHT button | 26 — INPUT_PULLUP |
| LEFT button | 27 — INPUT_PULLUP |
| UP button | 28 — INPUT_PULLUP |
| DOWN button LED | 18 — active LOW |
| RIGHT button LED | 19 — active LOW |
| LEFT button LED | 20 — active LOW |
| UP button LED | 21 — active LOW |

No LED matrix or potentiometer is used in this game.

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `MAX_LEN` | 20 | Sequence length needed to win |
| `MS_START` | 600 ms | Starting note duration |
| `MS_MIN` | 150 ms | Fastest note duration |
| `MS_REDUCE` | 20 ms | Speed-up per round |
| `MS_GAP` | 120 ms | Silence between notes |
| `MAX_LIVES` | 2 | Starting lives |
