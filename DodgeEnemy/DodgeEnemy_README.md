# Dodge Enemy

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Arcade / Reflex  
**Players:** 1

---

## How to Play

Enemies fall from the **top** of the 5×5 LED matrix toward the **bottom**, one row at a time.  
You are the single LED on the **bottom row**.  
Turn the **potentiometer** to move left and right — and dodge every enemy before it reaches your row!

When an enemy reaches the bottom row **and you are NOT in its column**, you score a point and that enemy resets to the top. Get hit and you lose a life.

---

## Controls

| Control | Pin | Action |
|---|---|---|
| Potentiometer | A1 | Move player left / right |
| LEFT / RIGHT button LEDs | 19, 20 | Mirror button state (visual feedback only) |

---

## Rules

- You start with **3 lives**.
- **Dodge** an enemy (it exits the bottom) → score **+1**, enemy respawns at the top.
- **Get hit** (enemy exits at your column) → lose 1 life, all enemies reset to the top. Your score is kept.
- At **score 10** a **3rd enemy** appears, making the game significantly harder.
- Speed increases by **10 ms per point**. Starting at 1000 ms/tick, minimum is 200 ms/tick (reached at score 80).
- Lose all 3 lives → **Game Over**. Your score (and high score if different) is shown on the matrix, then the game restarts.

---

## Enemies

| Enemy | Start position | Behavior |
|---|---|---|
| Enemy 1 | Row 4 (top) | Main enemy; scoring enemy — dodging it gives +1 point |
| Enemy 2 | 4 ticks behind Enemy 1 | Staggered so both don't arrive together; no extra score |
| Enemy 3 | Activates at score 10 | Appears 2 ticks off-screen; stays active for the rest of the game |

---

## RGB Warning Indicator

| Color | Meaning |
|---|---|
| Off | No immediate danger |
| 🟡 Yellow | An enemy is in your exact column |
| 🔴 Red | An enemy is 1 row away — dodge immediately! |

---

## Score Display

At game over, your current score is shown for 3 seconds (1 LED = 1 point, fills left-to-right bottom-to-top, max 25). If your all-time session high score is higher than your current score, it is shown afterwards.

---

## Countdown

A 3-2-1 countdown bar appears on the middle row at the start of every game (red → yellow → green), followed by a "GO!" flash.

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| Potentiometer | A1 |
| Left button | 27 — INPUT_PULLUP |
| Right button | 26 — INPUT_PULLUP |
| Left button LED | 20 — active LOW |
| Right button LED | 19 — active LOW |

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `TICK_START` | 1000 ms | Starting fall speed — higher = slower |
| `TICK_MIN` | 200 ms | Fastest possible speed |
| `TICK_REDUCE` | 10 ms | Speed gained (interval reduced) per point |
| `MAX_LIVES` | 3 | Starting lives |
| `THIRD_ENEMY_SCORE` | 10 | Score at which the 3rd enemy activates |
