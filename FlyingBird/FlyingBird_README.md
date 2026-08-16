# Flying Bird

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Arcade / Reflex  
**Players:** 1  
**Designed for:** Elementary school children

---

## How to Play

A wall descends from the top of the 5×5 matrix toward the bottom, one row at a time. The wall has **one dark gap** — the safe passage. Your bird (the blinking LED) is always on the bottom row.

Turn the **potentiometer** to slide your bird left and right and line it up with the gap **before the wall arrives**.

When the wall hits row 0:
- **Bird in the gap** → score a point! New wall spawns at the top.
- **Bird NOT in the gap** → lose a life! New wall spawns after the animation.

---

## Controls

| Control | Pin | Action |
|---|---|---|
| Potentiometer | A1 | Move bird left / right |

No buttons are needed. The LEFT and RIGHT buttons are connected but unused.

---

## Rules

- You start with **3 lives**.
- Successfully aligning with the gap → score **+1**.
- Missing the gap → lose 1 life. A new random wall spawns.
- Lose all 3 lives → Game Over. High score is shown, then auto-restart.
- The wall gets slightly faster every point scored.

---

## RGB Alignment Hint

The RGB LED gives you real-time feedback as the wall approaches:

| Wall distance | Bird aligned with gap | RGB |
|---|---|---|
| 3+ rows away | — | Off |
| 2 rows away | Yes | 🟢 Green — good position |
| 2 rows away | No | 🟡 Yellow — move now |
| 1 row away | Yes | 🟢 Green — stay here! |
| 1 row away | No | 🔴 Red — wrong position! |

---

## Speed

The wall takes one step every `WALL_MS` milliseconds (starts at **6000 ms**). Each point scored reduces the step time by 8 ms. Minimum step time is **2000 ms** (reached at score 500).

The full trip from top (row 4) to bottom (row 0) takes **4 steps**, so:
- Start: 4 × 6 s = ~24 s per wall
- Score 100: 4 × 5.2 s = ~21 s per wall
- Maximum speed: 4 × 2 s = 8 s per wall

The game is intentionally slow so young players have time to think.

---

## Tutorial (First Game Only)

The very first time the game starts, a brief tutorial runs automatically:

1. **"This is you"** — the blinking bird appears and follows the pot so the player can see the control.
2. **"Watch the gap"** — a wall is shown at row 3 with the gap at centre and the bird aligned; RGB turns green.
3. **Wall descends** — the wall steps down row by row so the player sees how it moves.
4. **"Wrong position"** — the bird is shown off-centre while the wall approaches; RGB turns yellow then red, collision animation plays.

After the tutorial the game starts normally.

---

## Score Display

At game over, the session high score is shown on the matrix for 3 seconds: one lit LED per point, filling left-to-right bottom-to-top (max 25 shown).

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| Potentiometer | A1 |

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `WALL_MS` | 6000 ms | Starting wall speed — higher = slower |
| `WALL_MS_MIN` | 2000 ms | Fastest wall speed |
| `SPEED_REDUCE` | 8 ms | Speed gain (interval cut) per point |
| `MAX_LIVES` | 3 | Starting lives |
| `BLINK_MS` | 500 ms | Bird blink rate |
