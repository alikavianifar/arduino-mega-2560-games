# Snake

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Classic Arcade  
**Players:** 1

---

## How to Play

Guide the snake around the 5×5 LED matrix using the four directional buttons. Eat the **blinking food LED** to grow longer. Don't run into your own body — the game ends if you do. The snake wraps around all four edges (exits one side, enters the opposite side).

---

## Controls

| Control | Pin | Action |
|---|---|---|
| UP button (blue) | 28 | Move up |
| DOWN button (green) | 25 | Move down |
| LEFT button (white) | 27 | Move left |
| RIGHT button (red) | 26 | Move right |
| Potentiometer | A1 | Set base speed (left=slow, right=fast) |

**Button LEDs** (pins 18–21) light up when the corresponding button is pressed — visual feedback only.

---

## Rules

- The snake starts with length 2, moving right.
- **Eat the blinking LED** → snake grows by 1.
- **Hit your own body** → Game Over (red RGB flash + low tone), then auto-restart.
- You **cannot reverse** direction (e.g., if moving right, pressing left does nothing).
- The snake **wraps around** all four edges — no wall collisions.
- There is **no score display** — the game restarts automatically after death.

---

## Speed

Speed is controlled by two factors:

**Potentiometer:** turning it right makes the base interval shorter (faster). Range: 800 ms (slow) → 150 ms (fast).

**Snake length:** every extra segment past 2 reduces the interval by an additional 20 ms, so the snake automatically speeds up as it grows.

The minimum interval is capped at **100 ms** to keep the game playable.

---

## RGB Indicator

| Color | Event |
|---|---|
| Off | Normal play |
| 🟢 Green | Flash when food is eaten |
| 🔴 Red | Flash on game over |

---

## Food Blinking

The food LED **blinks** (toggles every 300 ms) so it is easy to tell apart from the snake body on the matrix.

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| Potentiometer | A1 |
| UP button | 28 — INPUT_PULLUP |
| DOWN button | 25 — INPUT_PULLUP |
| LEFT button | 27 — INPUT_PULLUP |
| RIGHT button | 26 — INPUT_PULLUP |
| UP button LED | 21 — active LOW |
| DOWN button LED | 18 — active LOW |
| LEFT button LED | 20 — active LOW |
| RIGHT button LED | 19 — active LOW |

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `MIN_INTERVAL` | 100 ms | Minimum move interval (speed cap) |
| `FOOD_BLINK_MS` | 300 ms | Food LED blink rate |
| `bonus` formula | 20 ms × (len−2) | Extra speed per body segment |
