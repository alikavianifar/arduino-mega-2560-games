# Laser Corridor

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Puzzle / Strategy  
**Players:** 1  
**Alias:** Secret Agent

---

## Story

You are a secret agent infiltrating a facility. Five floors of laser beams stand between you and your goal. Each laser has one safe gap — find it and slip through. One wrong step and you start over!

---

## How to Play

1. Use the **potentiometer** to move your agent (single LED) left and right along the bottom row.
2. Watch the **RGB LED** — it tells you whether your current position is safe or not (see below).
3. Press the **button** to commit your position and attempt to pass through the laser.
4. If you chose the correct safe column → you advance to the next floor.
5. If you chose wrong → lasers trigger, you restart from floor 0 with a new layout.
6. Clear all **5 floors** to win!

---

## Controls

| Control | Pin | Action |
|---|---|---|
| Potentiometer | A1 | Move agent left / right |
| DOWN button | 25 | Commit position / restart after win |

---

## RGB Colour Hints

Each floor has exactly **2 columns highlighted in blue** (one truly safe, one decoy) and **3 red columns** (certain death).

| RGB Color | Meaning |
|---|---|
| 🔵 Blue | You are in the **safe zone** — but it's 50/50! One of the 2 blue columns is real, one is a trap. |
| 🔴 Red | You are **definitely on a laser**. Move before pressing! |
| 🟢 Green (solid) | You won! Press the button to play again. |

**Strategy:** Always move into the blue zone first, then make your 50/50 guess.

---

## Matrix Display

| LED state | Meaning |
|---|---|
| Fully lit rows (below agent) | Floors you have already cleared |
| Single LED | Your agent's current position |
| All LEDs lit + green RGB | Victory screen — press button to restart |

---

## Scoring

There is no score — the challenge is simply to reach the top. Each run generates a completely **new random laser layout**, so memorising previous solutions doesn't help.

---

## Game Over

Landing on any column that is NOT the exact safe column (including the blue decoy!) triggers the laser. Red RGB flashes, the buzzer sounds a low tone, and the game resets to floor 0 with a freshly generated layout.

---

## Hardware Pins

| Component | Pin |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| Potentiometer | A1 |
| Submit button | 25 — INPUT_PULLUP (active LOW) |

---

## Tunable Constants

| Item | Location in code | Effect |
|---|---|---|
| Number of floors | `currentFloor > 4` → change 4 | More floors = harder |
| Blue zone width | `hintCol` logic in `generateLasers()` | Currently always 2 columns (50/50) |
| Laser hit delay | `delay(400)` after hit | How long the red flash lasts |
