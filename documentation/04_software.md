# 04 — Software

## 1. Layout & responsibilities

| File | Responsibility |
|---|---|
| `firmware/solar_health_monitor/soiling_logic.h` | **Tested core**: soiling math, validity gate, 5-min rolling window, alert FSM. Pure C++ — no Arduino includes, so the *same* code runs in the firmware and in the PC tests. |
| `firmware/solar_health_monitor/config.h` | Pins, timings, Wi-Fi/cloud settings — the only file you edit before flashing. |
| `firmware/solar_health_monitor/solar_health_monitor.ino` | Thin glue: read sensors (8-sample average) → feed the engine → print status → upload every 20 s → serial console. |
| `tests/test_soiling_logic.cpp` | 40 assertions over the core (cases T1-T14). |
| `tests/test_mock_cloud.py` | 6 tests over the mock cloud endpoints. |
| `mock_cloud/server.py` | Local "cloud": ThingSpeak-style `/update`, JSON `/data`, live dashboard `/`. |
| `tools/i2c_scanner/i2c_scanner.ino` | Bus scanner for hardware bring-up. |

## 2. Algorithms (implemented exactly once, in `soiling_logic.h`)

| Step | Rule |
|---|---|
| Average | 8 samples per INA219 reading (15 ms apart) |
| Power | `P = V × I` (computed in the .ino for log/upload) |
| Validity gate | `valid = lux > 20 000 && I_ref > 5 mA` → if false: no math, no window, no state change |
| Soiling | `soil = (1 − (I_test/I_ref)/k) × 100`, clamped to [−10, 100]; guards for zero current/k |
| Rolling window | 15 samples, **one push per 20 s** (= 5 minutes), circular buffer, mean over the window |
| Alert ON | window **full** AND mean > 15 % |
| Alert OFF | mean < 8 % (values in 8-15 % **hold** the previous state → no flicker) |
| Fault | `I_ref > 50 mA && I_test < 5 mA` (priority over soiling) |
| Overheat | `tempC > 65` (priority over soiling) |
| Staleness | no valid reading for 10 min → window + alert forgotten (needs a fresh 5 min) |
| Calibration | `k = I_test/I_ref` with both panels clean, stored in NVS flash (`Preferences`) |

## 3. Constants

Thresholds live **only** in `solar::Config` (`soiling_logic.h`): `luxMin 20000`,
`soilAlertOn 15`, `soilAlertOff 8`, `tempWarnC 65`, `iRefMinMa 5`, `iRefFaultMa 50`,
`iTestFaultMa 5`, `windowSize 15`, `windowIntervalMs 20000`, `staleMs 600000`.
Firmware timing/pins/credentials: `config.h` (`READ_MS 5000`, `UPLOAD_MS 20000`,
`WIFI_RETRY_MS 60000`, `N_SAMPLES 8`, pins, `WIFI_SSID/PASS`, `CLOUD_URL`).

## 4. Running the tests (PC, no hardware)

```bash
# core logic (40 checks)
g++ -std=c++11 -Wall -Wextra -o tests/test_soiling_logic.exe tests/test_soiling_logic.cpp
./tests/test_soiling_logic.exe

# mock cloud (6 tests)
python -m unittest discover -s tests -p "test_*.py" -v
```

Both must print PASSED/OK before any firmware change is considered done.

## 5. Running the mock cloud

```bash
python mock_cloud/server.py
# dashboard : http://localhost:5000/
# raw JSON  : http://localhost:5000/data
# upload API: GET/POST /update?field1=6.01&field2=486&...&field7=0
```
Field mapping is documented in `02_system_design.md` § 5.

## 6. Building / flashing the firmware

1. Edit `config.h`: `WIFI_SSID`, `WIFI_PASS`, `CLOUD_URL` (your PC's LAN IP, e.g.
   `http://192.168.1.5:5000/update`).
2. Arduino IDE: board **ESP32 Dev Module**, open `solar_health_monitor.ino`, install
   libraries *Adafruit INA219*, *BH1750*, *OneWire*, *DallasTemperature*, upload.
   (Command line: `arduino-cli compile --fqbn esp32:esp32:esp32 firmware/solar_health_monitor`)
3. Serial Monitor at **115200** → you should see one status line every 5 s and
   `[cloud] ok` every 20 s when Wi-Fi works.

## 7. Serial console

| Key | Action |
|---|---|
| `c` | calibrate `k` — both panels must be clean, in strong sun (> 20 000 lx) |
| `t` | toggle **test mode** (fake readings → soiling 20 %, proves the pipeline with no sun) |
| `p` | print one status line now |
| `x` | erase stored calibration |
| `r` | reset the 5-minute window |

Status line format:
```
[120] VT=6.01 IT=486 VR=6.03 IR=494 P=2.92 lux=42150 T=41.6 soil=1.6 avg=1.4 valid=1 state=0 wifi=1
```

## 8. v2 backlog (deliberately not built)

OLED UI · buzzer/LED · relay/pump · ThingSpeak/Blynk switch (only `CLOUD_URL` changes) ·
daily Wh energy · OTA
