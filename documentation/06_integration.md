# 06 — Integration: How the Parts Work Together

## 1. The combined loop

```
hardware:  sun → panels → INA219 shunts ──I2C──▶ ESP32 ──Wi-Fi──▶ mock cloud → dashboard
software:           read 8× average      core: gate → soiling % → 5-min mean → FSM → field1..7
                                                                        │
                                              serial line every 5 s ◀───┘
```

| # | Hardware does | Software does | Result |
|---|---|---|---|
| 1 | both panels drive identical loads | — | physical currents |
| 2 | I2C/1-Wire carry digits | drivers average 8 samples | V, mA, lx, °C |
| 3 | ESP32 CPU runs | gate → soiling % → window | trustworthy soil % |
| 4 | — | FSM (fault > overheat > 15/8 %) | `state` 0-3 + alarm flag |
| 5 | USB serial / (v2 OLED+buzzer) | `printStatus()` | on-site status |
| 6 | Wi-Fi radio → router | HTTP GET every 20 s | record in the cloud |
| 7 | phone/PC browser | dashboard renders history | graphs + alert banner |

**Degrades gracefully:** no Wi-Fi → steps 1-5 still run; no sun → step 3 skips
evaluation; sensor dead → error printed, node keeps running.

## 2. Bring-up order (test one layer before adding the next)

| # | Layer | Pass test |
|---|---|---|
| 1 | Logic only | `./tests/test_soiling_logic.exe` → 40/40 (works with zero hardware) |
| 2 | Cloud only | `python mock_cloud/server.py` + browser shows `/update` posts |
| 3 | Firmware compile | `arduino-cli compile ...` → no errors |
| 4 | Board alone | Blink + serial banner at 115200 |
| 5 | Bus | I2C scanner finds 0x23, 0x40, 0x41 |
| 6 | Sensors | serial line matches multimeter/thermometer |
| 7 | Calibration | serial `c` in strong sun with clean panels → `k` 0.95-1.05 |
| 8 | Alert logic | serial `t` (test mode) → after 5 min `state=1` + `[cloud] ok` |
| 9 | End-to-end | dashboard shows the 20 % spike; wipe/`t` off → returns to 0 |
| 10 | Outdoor soak | ≥ 24 h run, no resets, dust test (flour/soil) → alert → clean → clear |

## 3. Firmware ⇄ cloud contract

- Transport: `GET CLOUD_URL?field1..field7` (ThingSpeak-compatible).
- To move to **real ThingSpeak**: create a channel (7 fields), then in `config.h` set
  `CLOUD_URL = "https://api.thingspeak.com/update?api_key=YOUR_KEY"`.
- The dashboard polls `/data` every 5 s; state names: 0 OK · 1 CLEANING · 2 FAULT ·
  3 OVERHEAT.

## 4. Common integration problems

| Symptom | Cause | Fix |
|---|---|---|
| `[hw] INA219 ... NOT found` | wiring / both at 0x40 | check SDA/SCL/GND; bridge A0→VCC on the ref unit |
| Current negative | shunt backwards | swap VIN+/VIN− |
| `soil=0` always but dust visible | reference also dirty / low light | wipe reference; check `valid=0` |
| `valid=0` all day | lux ≤ 20 000 | check BH1750 orientation/shading |
| `[cloud] error -1` | wrong IP / server not running / different network | run `server.py`, use PC LAN IP, same Wi-Fi |
| `[cloud] skipped (no wifi)` | 5 GHz SSID or wrong password | ESP32 needs 2.4 GHz |
| alert never clears | window still holds old samples | press `r`, wipe reference, wait 5 min |

## 5. Final integration checklist

- [ ] 40/40 core tests · 6/6 cloud tests · firmware compiles
- [ ] Firmware reaches the mock cloud every 20 s (`[cloud] ok`)
- [ ] Test mode (`t`) raises `state=1` after 5 minutes and clears after `r`/clean input
- [ ] Hardware bring-up rows 4-10 passed (when parts arrive)
- [ ] 24 h soak with zero resets
