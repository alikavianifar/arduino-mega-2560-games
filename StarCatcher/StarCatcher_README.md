# Star Catcher

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Arcade / Reflex  
**Players:** 1

---

## How to Play

A star falls from the **top** of the 5×5 LED matrix toward the **bottom**, one row at a time.  
Your basket — 2 LEDs wide — sits at the bottom row.  
Use the **potentiometer** to slide the basket left and right and **catch the star** before it hits the ground.

Each time the star is caught, the game gets a little faster. Miss three times and it's game over.

---

## Controls

| Control | Pin | Action |
|---|---|---|
| Potentiometer | A1 | Move basket left / right |
| DOWN button | 25 | Pause / Resume |
| UP button | 28 | Restart game |

---

## Rules

- You start with **3 lives**.
- **Catch** the star → score +1, star falls a little faster next time.
- **Miss** the star → lose 1 life. The speed slows slightly as a short grace period.
- Every **8 seconds** the game also auto-speeds up, regardless of catches.
- Lose all 3 lives → **Game Over**. Your score is shown on the matrix (each lit LED = 1 point, max 25 shown), then the game restarts.

---

## RGB Indicator

The RGB LED always shows how fast the star is currently falling.

| Color | Meaning |
|---|---|
| 🔵 Blue | Slow — comfortable pace |
| 🟢 Green | Medium — getting faster |
| 🔴 Red | Fast — full focus needed! |

---

## Pause LED

The **DOWN button LED** (pin 18) lights up solid **blue** whenever the game is paused.  
Press DOWN again to resume — the LED turns off and the countdown timers resync from that moment.

---

## Countdown

At the start of every game (or after a restart), a 3-2-1 countdown appears on the middle row of the matrix with matching RGB colors (red → green → blue) and ascending tones. Two quick flashes signal "GO!"

---

## Score Display

When the game ends, your score is shown on the matrix for 2.5 seconds.  
One lit LED = 1 point. LEDs fill left-to-right, bottom-to-top (maximum 25 displayed).

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R = 9, G = 10, B = 11 (active HIGH) |
| Buzzer | 3 |
| Potentiometer | A1 |
| Pause button | 25 — INPUT_PULLUP (active LOW) |
| Restart button | 28 — INPUT_PULLUP (active LOW) |
| Pause LED | 18 — active LOW (lit = paused) |

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `SPEED_START` | 600 ms | Starting fall interval — higher = slower start |
| `SPEED_MIN` | 80 ms | Fastest possible interval — lower = more extreme top speed |
| `SPEED_CATCH` | 25 ms | Speed gained per catch |
| `SPEED_MISS` | 60 ms | Speed lost (grace) after a miss |
| `SPEED_AUTO` | 15 ms | Speed gained per automatic tick |
| `SPEED_INTERVAL` | 8000 ms | How often the auto-speedup fires |
| `BASKET_W` | 2 | Basket width in columns |
| `MAX_LIVES` | 3 | Starting lives |
