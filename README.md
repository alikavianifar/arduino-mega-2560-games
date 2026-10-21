# <div align="center">Arduino Games</div>

<p align="center">
A collection of **9 games and educational tools** built for the **Purple-Kit (Arduino Mega 2560)** — designed for elementary school students learning physical computing.

Every project uses the same 5×5 LED matrix, RGB LED, buzzer, potentiometer, buttons, and LDR that come with the Purple-Kit. No extra hardware required.
</p>

---

## Projects

| # | Name | Type | Controls |
|---|---|---|---|
| 1 | [Snake](#1-snake) | Classic arcade | 4 buttons + pot (speed) |
| 2 | [Simon Says](#2-simon-says) | Memory / rhythm | 4 buttons |
| 3 | [Star Catcher](#3-star-catcher) | Arcade / reflex | Pot |
| 4 | [Flying Bird](#4-flying-bird) | Arcade / reflex | Pot |
| 5 | [Dodge Enemy](#5-dodge-enemy) | Arcade / reflex | Pot |
| 6 | [Bomb Disposal](#6-bomb-disposal) | Puzzle / action | 4 buttons + LDR |
| 7 | [Laser Corridor](#7-laser-corridor) | Puzzle | Pot + 1 button |
| 8 | [Morse Code Translator](#8-morse-code-translator) | Educational tool | 4 buttons |
| 9 | [Light Meter](#9-light-meter) | Sensor display | LDR (automatic) |

---

## Hardware — Purple-Kit Pin Map

All projects share the same wiring. No changes needed between games — just upload a different sketch.

### 5×5 LED Matrix

```
        col0  col1  col2  col3  col4
row4:    33    34    43    44    53   ← TOP
row3:    32    35    42    45    52
row2:    31    36    41    46    51   ← MIDDLE
row1:    30    37    40    47    50
row0:    29    38    39    48    49   ← BOTTOM
```

> `row0 = BOTTOM`, `row4 = TOP`, `col0 = LEFT`, `col4 = RIGHT`

### Other Pins

| Component | Pin | Notes |
|---|---|---|
| RGB LED — Red | 9 | Active HIGH |
| RGB LED — Green | 10 | Active HIGH |
| RGB LED — Blue | 11 | Active HIGH |
| Buzzer | 3 | |
| Potentiometer | A1 | |
| LDR | A2 | Used in Bomb Disposal |
| LDR (calibration seed) | A0 | Used in Light Meter |
| Button UP (blue) | 28 | INPUT_PULLUP, active LOW |
| Button DOWN (green) | 25 | INPUT_PULLUP, active LOW |
| Button LEFT (white) | 27 | INPUT_PULLUP, active LOW |
| Button RIGHT (red) | 26 | INPUT_PULLUP, active LOW |
| Button LED UP | 21 | Active LOW |
| Button LED DOWN | 18 | Active LOW |
| Button LED LEFT | 20 | Active LOW |
| Button LED RIGHT | 19 | Active LOW |

---

## Getting Started

### Requirements

- Arduino IDE 1.8+ or Arduino IDE 2.x
- Board: **Arduino Mega 2560** (select in IDE: *Tools → Board → Arduino Mega 2560*)
- No external libraries required — all code uses standard Arduino functions

### Upload a Game

1. Open the `.cpp` file for the game you want in Arduino IDE.
2. Select the correct port: *Tools → Port → (your board's port)*
3. Click **Upload**.
4. The game starts automatically after upload.

> **Tip:** Each `.cpp` file is self-contained — every game has all its code in one file.

---

## Project Details

### 1. Snake

> `Snake/Snake.cpp` · `Snake/Snake_README.md`

Classic snake game on the 5×5 matrix. Eat the blinking food LED to grow. Don't bite yourself. The snake wraps around all four edges.

**Controls:** UP / DOWN / LEFT / RIGHT buttons for direction. Potentiometer sets base speed (slow → fast). The snake also speeds up slightly as it grows.

**Key features:** Food LED blinks to distinguish it from the snake body. Button LEDs mirror button state as visual feedback.

---

### 2. Simon Says

> `SimonSays/SimonSays.cpp` · `SimonSays/SimonSays_README.md`

The classic memory game. Simon flashes a sequence of button LEDs; you repeat it in the same order. Each round adds one more step.

**Controls:** DOWN / RIGHT / LEFT / UP buttons (each has a distinct tone: C4 / E4 / G4 / C5).

**Key features:** Clear two-phase design — green RGB while Simon shows the sequence (no input accepted), blue RGB when it's your turn. Combo milestone celebrations every 5 rounds. 2 lives.

---

### 3. Star Catcher

> `StarCatcher/StarCatcher.cpp` · `StarCatcher/StarCatcher_README.md`

A star falls from the top of the matrix. Move your 2-LED basket along the bottom row to catch it before it hits the ground.

**Controls:** Potentiometer (basket position). DOWN button = pause/resume. UP button = restart.

**Key features:** RGB shows current fall speed (blue / green / red). Speed increases with catches and auto-increases every 8 seconds. Pause LED indicator. 3 lives.

---

### 4. Flying Bird

> `FlyingBird/FlyingBird.cpp` · `FlyingBird/FlyingBird_README.md`

A wall descends row by row. It has one gap. Align your blinking bird (always on the bottom row) with the gap before the wall arrives.

**Controls:** Potentiometer only.

**Key features:** Built-in tutorial on the very first game that shows how the mechanics work. RGB gives alignment hints when the wall is 1–2 rows away. Intentionally slow-paced for young players.

---

### 5. Dodge Enemy

> `DodgeEnemy/DodgeEnemy.cpp` · `DodgeEnemy/DodgeEnemy_README.md`

Enemies fall from the top. You are the single LED on the bottom row. Dodge them — don't be in their column when they reach the bottom.

**Controls:** Potentiometer (player position).

**Key features:** Two enemies from the start (staggered). Third enemy appears at score 10. RGB warns: yellow = enemy in your column, red = 1 row away. Speed increases with score. 3 lives.

---

### 6. Bomb Disposal

> `BombDisposal/BombDisposal.cpp` · `BombDisposal/BombDisposal_README.md`

Navigate your agent to a bomb on the 5×5 grid, then **cover the LDR sensor** to defuse it before the timer runs out. The bomb eventually starts moving, and a second bomb appears at high levels.

**Controls:** UP / DOWN / LEFT / RIGHT buttons. LDR sensor (cover with finger to defuse).

**Key features:** RGB countdown indicator (blue → green → yellow → red). Combo system rewards consecutive defuses. Moving bomb from level 5, double bomb from level 8. 3 lives.

---

### 7. Laser Corridor

> `LaserCorridor/LaserCorridor.cpp` · `LaserCorridor/LaserCorridor_README.md`

Navigate through 5 laser floors from bottom to top. Each floor has one safe column — but the RGB only shows you a 2-column "safe zone" (50/50 chance). Choose wisely and press to commit.

**Controls:** Potentiometer (column selection). DOWN button (commit / restart after win).

**Key features:** Blue RGB = you're in the safe zone (50/50). Red RGB = certain laser hit. New random layout every attempt. Cleared floors stay lit to show progress.

---

### 8. Morse Code Translator

> `MorseCode/MorseCode.cpp` · `MorseCode/MorseCode_README.md`

An educational tool — not a game. Enter Morse code using dot and dash buttons. The Arduino translates the sequence and displays the letter as a 5×5 pixel character on the matrix. Output also appears in the Serial Monitor.

**Controls:** WHITE = dot (·) · GREEN = dash (−) · RED = space/word gap · BLUE = backspace.

**Key features:** Auto-translates after 1 second of silence. RGB feedback for each action (green = success, red = unknown, yellow = space, blue = backspace). Full A–Z bitmap font. Serial Monitor at 9600 baud shows decoded text in real time.

---

### 9. Light Meter

> `LightMeter/LightMeter.cpp` · `LightMeter/LightMeter_README.md`

A sensor display tool. Reads ambient light from the LDR and shows a live bar graph on the matrix. Auto-calibrates to the range of light seen since power-on.

**Controls:** None — fully automatic.

**Key features:** 0–5 row bar graph (bottom = dark, top = bright). RGB colour reflects level (white / blue / green / red). Serial Monitor at 9600 baud shows raw LDR value, calibrated range, and level in real time. Great for classroom demonstrations.

---

## Repository Structure

```
purple-kit-games/
│
├── README.md                        ← you are here
│
├── Snake/
│   ├── Snake.cpp
│   └── Snake_README.md
│
├── SimonSays/
│   ├── SimonSays.cpp
│   └── SimonSays_README.md
│
├── StarCatcher/
│   ├── StarCatcher.cpp
│   └── StarCatcher_README.md
│
├── FlyingBird/
│   ├── FlyingBird.cpp
│   └── FlyingBird_README.md
│
├── DodgeEnemy/
│   ├── DodgeEnemy.cpp
│   └── DodgeEnemy_README.md
│
├── BombDisposal/
│   ├── BombDisposal.cpp
│   └── BombDisposal_README.md
│
├── LaserCorridor/
│   ├── LaserCorridor.cpp
│   └── LaserCorridor_README.md
│
├── MorseCode/
│   ├── MorseCode.cpp
│   └── MorseCode_README.md
│
└── LightMeter/
    ├── LightMeter.cpp
    └── LightMeter_README.md
```

---

## Design Principles

- **One file per project** — no libraries to install, no dependencies.
- **Non-blocking timing** — all games use `millis()` instead of `delay()` for game logic, keeping the loop responsive.
- **Debounced buttons** — edge-detect logic prevents accidental double-presses.
- **Consistent pin map** — every project uses the same `MATRIX[5][5]` layout and pin assignments, making the code easy to read across projects.
- **Classroom-friendly** — intentionally slow default speeds, clear RGB feedback, and tutorial modes where appropriate.

---

## License

MIT License — free to use, modify, and share.
