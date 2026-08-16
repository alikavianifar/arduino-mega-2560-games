# Morse Code Translator

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Type:** Educational Tool  
**Players:** 1

---

## What It Does

Enter Morse code using the dot and dash buttons. The Arduino translates the sequence into a letter and displays it as a 5×5 pixel character on the LED matrix. Decoded letters also appear in the **Serial Monitor** (9600 baud) so you can read the full word on a computer.

---

## Controls

| Button | Color | Pin | Action |
|---|---|---|---|
| DOT | White | 27 | Add a `.` to the current sequence |
| DASH | Green | 25 | Add a `-` to the current sequence |
| SPACE | Red | 26 | Translate now + insert a word space in Serial |
| BACKSPACE | Blue | 28 | Delete the last symbol entered |

**Button LEDs** (pins 18–21) light up when the corresponding button is pressed.

---

## How to Type a Letter

1. Press **DOT** and/or **DASH** in the correct Morse order for the letter you want.
2. Either press **SPACE** to translate immediately, or simply wait **1 second** — the translator auto-decodes after a 1 s pause.
3. The letter appears on the **LED matrix** and in the **Serial Monitor**.
4. Start entering the next letter. The matrix clears automatically when you press the first button of a new sequence.

To type a **word space**, press **SPACE** when no sequence is in progress.

---

## RGB Feedback

| Color | Meaning |
|---|---|
| 🟢 Green flash | Letter decoded successfully |
| 🔴 Red flash | Unknown sequence → `?` displayed |
| 🟡 Yellow flash | Space / word gap entered |
| 🔵 Blue flash | Backspace used |

---

## Morse Code Reference (A-Z)

```
A .-      B -...    C -.-.    D -..     E .
F ..-.    G --.     H ....    I ..      J .---
K -.-     L .-..    M --      N -.      O ---
P .--.    Q --.-    R .-.     S ...     T -
U ..-     V ...-    W .--     X -..-    Y -.--
Z --..
```

---

## Auto-Translate

If you stop pressing buttons for **1 second** after entering symbols, the sequence is translated automatically — no need to press SPACE.

---

## Serial Monitor

Connect via USB and open the Serial Monitor at **9600 baud** to see the full decoded text as you type. Backspace shows the remaining sequence in brackets (e.g. `[.-]` after deleting one symbol from `[.--]`).

---

## Letter Display on Matrix

Each letter is rendered as a **5×5 pixel** shape on the LED matrix. If a Morse sequence doesn't match any letter, a `?` shape is shown and the RGB flashes red.

---

## Hardware Pins

| Component | Pin(s) |
|---|---|
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| Buzzer | 3 |
| DOT button | 27 — INPUT_PULLUP |
| DASH button | 25 — INPUT_PULLUP |
| SPACE button | 26 — INPUT_PULLUP |
| BACKSPACE button | 28 — INPUT_PULLUP |
| DOT LED | 20 — active LOW |
| DASH LED | 18 — active LOW |
| SPACE LED | 19 — active LOW |
| BACKSPACE LED | 21 — active LOW |

---

## Tunable Constant

| Constant | Default | Effect |
|---|---|---|
| `TIMEOUT` | 1000 ms | Silence before auto-translate fires |
