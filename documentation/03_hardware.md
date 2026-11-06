# 03 — Hardware

> Status: **specification complete, parts not purchased yet.** This is the buy/wire/build
> sheet for when they arrive. Full diagrams and theory: *Complete Project Details.md*.

## 1. Bill of Materials

| # | Component | Key spec | Qty | ≈ INR |
|---|---|---|---|---|
| 1 | ESP32 DevKit (WROOM, 30/38 pin) | 2.4 GHz Wi-Fi, 3.3 V | 1 | 430-500 |
| 2 | Solar panels, **identical pair** | 6 V / 2-3 W, **Voc < 26 V** | 2 | 500-1,000 |
| 3 | INA219 module | 0-26 V, 0.1 Ω shunt, I2C | 2 | 250-400 |
| 4 | BH1750 (GY-302) | lux 1-65 535 | 1 | 70-150 |
| 5 | DS18B20 waterproof probe + 4.7 kΩ | ±0.5 °C, 1-Wire | 1 | 80-150 |
| 6 | Load resistors | R = Vmp/Imp (e.g. 12 Ω for 6 V/0.5 A), **≥ 5-10 W** | 2 | 20-60 |
| 7 | 5 V / ≥1 A USB adapter | powers ESP32 only | 1 | 100-300 |
| 8 | Enclosure + glands + desiccant, frame, wires, board | IP54-ish, fixed tilt | 1 set | 500-1,000 |
| — | *v2 backlog:* OLED SSD1306, buzzer, LED, relay, pump | | | |

**Total ≈ Rs 2,000-3,000 basic / Rs 3,000-4,500 recommended.** Buy both panels from the
same batch. Check prices before ordering.

## 2. Pin map & addresses

| Function | ESP32 | Note |
|---|---|---|
| I2C SDA / SCL | GPIO21 / GPIO22 | shared by all I2C devices, 3.3 V |
| DS18B20 data | GPIO4 | needs **4.7 kΩ pull-up to 3.3 V** |
| *(v2)* buzzer / LED / relay | GPIO25 / 26 / 27 | not used in v1 |
| **Never use** | GPIO 6-11 (flash), 0/2/12/15 (strap), 34-39 (input-only) | breaks boot |

| Device | Address | Set by |
|---|---|---|
| INA219 test panel | 0x40 | default (A0→GND) |
| INA219 reference | 0x41 | **bridge A0 pad → VCC** on the 2nd module |
| BH1750 | 0x23 | ADDR low (default) |
| OLED (v2) | 0x3C | module default |

## 3. Wiring (every connection)

| From | To |
|---|---|
| USB adapter 5 V | ESP32 5V/USB |
| ESP32 GND | common ground rail |
| 3.3 V rail | INA219 ×2 VCC, BH1750 VCC, DS18B20 VDD |
| GPIO21 / GPIO22 | common SDA / SCL to all I2C devices |
| DS18B20 DATA | GPIO4 + 4.7 kΩ to 3.3 V |
| Test panel (+) | INA219 #1 **VIN+** → VIN− → load resistor → GND |
| Ref panel (+) | INA219 #2 **VIN+** → VIN− → load resistor → GND |
| Panel (−) | common GND |

Rules: **never power the ESP32 from the panels** · same wire gauge/length both sides ·
one shared ground · shunt polarity matters (negative reading → swap VIN+/VIN−).

## 4. Mechanical

- Both panels **same tilt (≈ local latitude) and azimuth**, gap ≥ one panel height, no
  mutual shading at sunrise/sunset.
- BH1750 flat with clear sky view; DS18B20 taped to the **back** of the test panel.
- Enclosure shaded, ventilated, cable glands + strain relief + silica gel.
- **Reference panel discipline:** wipe weekly with a dry cloth and log the date — if it
  gets dirty the index drifts toward 0 (false "all clean").

## 5. Assembly order (each step must pass before the next)

1. ESP32 alone → Blink + Serial (115200)
2. I2C devices → run `tools/i2c_scanner` → expect **0x23, 0x40, 0x41** (+0x3C if OLED)
3. DS18B20 → reading matches a thermometer (never −127)
4. Panels → INA219 → loads → readings match a multimeter (±2 % V, ±5 % I)
5. Mount frame, add strain relief, enclose
6. Both panels clean + strong sun → firmware serial `c` → `k` = 0.95-1.05 stored

## 6. Hardware acceptance checklist

- [ ] I2C scan: 0x23, 0x40, 0x41 (+0x3C with OLED)
- [ ] V/I within a few % of a multimeter; temperature within 1 °C
- [ ] `k` stored and 0.95-1.05; survives reboot
- [ ] No resets during a 24 h run; wiring survives being moved
