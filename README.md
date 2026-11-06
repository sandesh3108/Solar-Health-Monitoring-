# Solar Health Monitoring

**Smart Solar Panel Health & Dust Monitoring System with Wireless Telemetry** — an ESP32
node that compares a *dusty* test panel against a *clean* reference panel, computes a
soiling index (%), and reports/alerts over Wi-Fi.

## Repository layout

| Path | What |
|---|---|
| `firmware/solar_health_monitor/` | ESP32 firmware (`.ino` + tested core `soiling_logic.h` + `config.h`) |
| `tests/` | PC unit tests: core logic (g++) and mock cloud (Python) |
| `mock_cloud/` | Local ThingSpeak-style cloud + live dashboard (Flask) |
| `tools/i2c_scanner/` | I2C bus scanner for hardware bring-up |
| `documentation/` | **All docs, one file per part** (start at `documentation/00_index.md`) |

## Quick start (no hardware needed)

```bash
# 1. run the core-logic tests (40 checks)
g++ -std=c++11 -Wall -Wextra -o tests/test_soiling_logic.exe tests/test_soiling_logic.cpp
./tests/test_soiling_logic.exe

# 2. run the mock-cloud tests (6 tests)
python -m unittest discover -s tests -p "test_*.py" -v

# 3. start the local cloud/dashboard
python mock_cloud/server.py        # http://localhost:5000/

# 4. build the firmware (needs arduino-cli + esp32 core)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/solar_health_monitor
```

Before flashing: edit `firmware/solar_health_monitor/config.h`
(`WIFI_SSID`, `WIFI_PASS`, `CLOUD_URL`).

## Key logic

```
Soiling % = (1 − (I_test / I_ref) / k) × 100      // k = calibration (both panels clean)
valid     = lux > 20 000 && I_ref > 5 mA           // no false alerts at night/cloud
ALERT     when 5-min mean > 15 %  (full window)    // cleared when mean < 8 %
FAULT     when I_ref > 50 mA && I_test < 5 mA      // OVERHEAT when T > 65 °C
```

See `documentation/00_index.md` for the full documentation map.
