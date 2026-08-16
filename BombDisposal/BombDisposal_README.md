# Bomb Disposal

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Genre:** Puzzle / Action  
**Players:** 1

---

## How to Play

A bomb appears somewhere on the 5×5 LED matrix. You are the **blinking LED**.

1. Use the **four directional buttons** to navigate to the bomb's position.
2. Once you are standing on the bomb, **cover the LDR sensor** (pin A2) with your finger or a dark object.
3. If the LDR reads darkness, the bomb is defused → score +1, new bomb spawns.
4. If the **timer runs out** before you defuse it → explosion! Lose a life.
5. Survive with 3 lives and defuse as many bombs as possible!

---

## Controls

| Control | Pin | Action |
|---|---|---|
| UP button | 28 | Move player up |
| DOWN button | 25 | Move player down |
| LEFT button | 27 | Move player left |
| RIGHT button | 26 | Move player right |
| LDR (light sensor) | A2 | **Cover it** to defuse when standing on bomb |

Button LEDs (pins 18–21) light up when the corresponding button is pressed.

---

## Rules

- You start with **3 lives**.
- **Defuse** a bomb (reach it + cover LDR) → score +1, bomb respawns elsewhere.
- **Timer expires** → explosion, lose 1 life. After each explosion the timer gets slightly longer as a grace period. After each defuse it gets shorter (faster!).
- Lose all 3 lives → Game Over. Score is shown on the matrix, then auto-restart.

---

## Combo System

Defuse **3 or more bombs in a row** without letting one explode = **Combo!**  
On a combo you earn a **+1 bonus point** and hear a special fanfare.  
Any explosion resets the combo counter to 0.

---

## Level Progression

| Score | Level | New Feature |
|---|---|---|
| 0–4 | 1–4 | One static bomb |
| 5–9 | → Lv 5 | **Bomb starts moving** randomly every 2.2 seconds |
| 10–14 | → Lv 6–7 | Still one moving bomb, faster timer |
| 15+ | → Lv 8+ | **Second bomb appears** simultaneously |

Levels increase every 5 successful defuses. Each level triggers a column-sweep animation and a rising fanfare.

---

## RGB Countdown Indicator

The RGB LED shows how much time is left on the current bomb:

| Color | Remaining time |
|---|---|
| 🔵 Blue | More than 60% — comfortable |
| 🟢 Green | 35–60% — move! |
| 🟡 Yellow | 15–35% — urgent! |
| 🔴 Red | Under 15% — critical! |

---

## Score Display

At game over, score is shown for 2.5 seconds: each lit LED = 1 point, filling the matrix left-to-right bottom-to-top (max 25 shown).

---

## Visual Cues

| LED behaviour | Meaning |
|---|---|
| Blinking LED | Your player |
| Steady LED | Bomb |
| Player blink stops (steady) | You are standing on the bomb — now cover the LDR! |

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| LDR sensor | A2 (cover = HIGH reading = defuse) |
| UP button | 28 — INPUT_PULLUP |
| DOWN button | 25 — INPUT_PULLUP |
| LEFT button | 27 — INPUT_PULLUP |
| RIGHT button | 26 — INPUT_PULLUP |
| UP LED | 21 — active LOW |
| DOWN LED | 18 — active LOW |
| LEFT LED | 20 — active LOW |
| RIGHT LED | 19 — active LOW |

---

## Game Constants (easy to tune)

| Constant | Default | Effect |
|---|---|---|
| `TIME_START` | 5000 ms | Starting bomb timer |
| `TIME_MIN` | 900 ms | Fastest possible timer |
| `TIME_DEFUSE` | 200 ms | Timer reduction per defuse |
| `TIME_EXPLODE` | 400 ms | Timer increase after explosion (grace) |
| `LDR_THRESH` | 520 | Cover-darkness threshold |
| `MOVE_LEVEL` | 5 | Level when bomb starts moving |
| `DOUBLE_LEVEL` | 8 | Level when 2nd bomb appears |
| `BOMB_MOVE_MS` | 2200 ms | How often the moving bomb shifts |
| `MAX_LIVES` | 3 | Starting lives |
