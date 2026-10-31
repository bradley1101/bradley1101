# Wiring — Smart Home prototype

All nodes use an **ESP32 DevKit V1** (30 pins). Pin numbers are GPIO numbers and match [`firmware/include/config.h`](../firmware/include/config.h).

**Power rules**

- Servos (SG90) and the HC-SR04 run on **5 V** (ESP32 `VIN` when powered by USB, or an external 5 V supply for several servos).
- The RC522 runs on **3.3 V only** — 5 V destroys it.
- **All grounds are common** (ESP32, sensors, servos, external supply).
- The ESP32 inputs are 3.3 V: the HC-SR04 `ECHO` (5 V) goes through a divider **1 kΩ (series) + 2 kΩ (to GND)**.
- Each LED has a **220 Ω** series resistor.

---

## 1. Entrance node — access control & door

| Component | Component pin | ESP32 |
| --- | --- | --- |
| RC522 RFID | SDA (SS) | GPIO 5 |
| | SCK | GPIO 18 |
| | MOSI | GPIO 23 |
| | MISO | GPIO 19 |
| | RST | GPIO 22 |
| | 3.3V / GND | 3V3 / GND |
| SG90 servo (door) | Signal (orange) | GPIO 13 |
| | VCC (red) / GND (brown) | 5V / GND |
| Green LED (granted) | Anode → 220 Ω | GPIO 26 |
| Red LED (denied) | Anode → 220 Ω | GPIO 27 |
| HC-SR04 (visitor) | TRIG | GPIO 32 |
| | ECHO → divider 1 kΩ / 2 kΩ | GPIO 33 |
| | VCC / GND | 5V / GND |

## 2. Living room node — presence lighting

| Component | Component pin | ESP32 |
| --- | --- | --- |
| PIR HC-SR501 | OUT | GPIO 27 |
| | VCC / GND | 5V / GND |
| Room LEDs | Anode → 220 Ω (one per LED) | GPIO 25 |

> The HC-SR501 has two potentiometers (sensitivity, hold time). Set the hold time to the minimum: the firmware handles the delay (`LIGHT_HOLD_MS`).

## 3. Kitchen node — climate & ventilation

| Component | Component pin | ESP32 |
| --- | --- | --- |
| DHT11 (module, 3 pins) | DATA | GPIO 4 |
| | VCC / GND | 3V3 / GND |
| SG90 servo (window) | Signal (orange) | GPIO 13 |
| | VCC / GND | 5V / GND |
| Red LED (alert) | Anode → 220 Ω | GPIO 26 |

> A bare DHT11 (4 pins, no module) needs a 10 kΩ pull-up between DATA and VCC.

---

## Placement in the house

```
 ┌───────────────────────── 47 cm ─────────────────────────┐
 │  KITCHEN                │  LIVING ROOM                   │
 │  DHT11, alert LED       │  PIR on the wall, LEDs         │  28 cm
 │  window servo           │                                │
 ├─────────────────────────┴───────────┬────────────────────┤
 │  ENTRANCE: RC522 next to the door, door servo,           │
 │  HC-SR04 facing outside, green/red LEDs                  │
 └──────────────────────────────────────────────────────────┘
```
