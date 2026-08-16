# Light Meter

**Platform:** Purple-Kit (Arduino Mega 2560)  
**Type:** Sensor Display Tool  
**Sensor:** LDR (Light Dependent Resistor) on pin A0

---

## What It Does

Reads the ambient light level from the LDR and shows it as a **vertical bar graph** on the 5×5 LED matrix — the brighter the light, the more rows light up. The RGB LED changes colour to reflect the current level at a glance.

The tool **auto-calibrates**: it continuously tracks the darkest and brightest readings seen since power-on, and uses that range to fill the full bar. This means it automatically adapts to the room's lighting conditions.

---

## How to Use

1. Power on — the bar will settle within a second or two.
2. **Shine a torch** toward the LDR to see the bar rise and the RGB turn red.
3. **Cover the LDR** with your hand to see the bar drop and the RGB turn blue/white.
4. Open the **Serial Monitor** at **9600 baud** to read the exact values.

---

## Bar Graph Layout

| Rows lit | Light Level | RGB Color |
|---|---|---|
| 0 rows (dark) | Very dark | ⚪ White |
| 1–2 rows | Dim | 🔵 Blue |
| 3–4 rows | Moderate | 🟢 Green |
| 5 rows (all) | Bright | 🔴 Red |

Rows fill **from the bottom up** — row 0 is the base, row 4 is the top.

---

## Auto-Calibration

On startup the calibration range is empty. After a few seconds of normal use (with some light variation), the bar spreads naturally across the 5 rows. To **reset calibration**, simply restart the board.

The minimum allowed range is **10 ADC units** to prevent erratic behaviour in perfectly uniform light.

---

## Serial Monitor Output

Each update prints one line:

```
LDR: 542  [min=120 max=890]  Level: 3
```

- **LDR** — raw ADC reading (0–1023)
- **min / max** — calibrated range so far
- **Level** — bar height (0–5)

Update rate is ~10 times per second (100 ms delay).

---

## Hardware Pins

| Component | Pin |
|---|---|
| LDR | A0 (analog input) |
| RGB LED | R=9, G=10, B=11 (active HIGH) |
| LED Matrix | 29–53 (see `MATRIX[5][5]` in code) |

No buttons are used by this tool.
