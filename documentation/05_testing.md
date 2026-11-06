# 05 — Testing

Test-first rule applied throughout: **write the test → see it fail → implement → see it pass.**

## 0. How to run everything (PC, no hardware)

```bash
# core logic  (40 checks)
g++ -std=c++11 -Wall -Wextra -o tests/test_soiling_logic.exe tests/test_soiling_logic.cpp
./tests/test_soiling_logic.exe

# mock cloud  (6 tests)
python -m unittest discover -s tests -p "test_*.py" -v

# firmware compile check (needs arduino-cli + esp32 core)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/solar_health_monitor
```

## 1. Core logic — unit tests (PC, `g++`)

**Result: `40 checks, 0 failures -> PASSED`** (exit code 0, `-Wall -Wextra` clean)

| Case | What it proves | Result |
|---|---|---|
| T1 | clean panels (ratio 1.00) → soiling 0 % | ✅ |
| T2 | ratio 0.80 → soiling 20 % | ✅ |
| T3 | calibration `k=1.05` compensates panel mismatch (9.52 %) | ✅ |
| T4 | clamp at −10 % when test panel reads stronger than reference | ✅ |
| T5 | guards: zero reference current / zero k → 0 % (no NaN/div-by-zero) | ✅ |
| T5 | dead test panel → 100 % (clamped) | ✅ |
| T6 | lux < 20 000 → `valid=0`, no math, state untouched, window untouched | ✅ |
| T6b | I_ref ≤ 5 mA → `valid=0` | ✅ |
| T7 | 14 samples → window not full, no alert; 15th → full window + ALERT + mean 20 % | ✅ |
| T8 | mean < 8 % after cleaning → alert clears, state returns to OK | ✅ |
| T9 | mean inside 8–15 % **holds** the previous state (no flicker) | ✅ |
| T10 | I_ref > 50 mA & I_test < 5 mA → FAULT | ✅ |
| T11 | T > 65 °C → OVERHEAT priority; cools down → back to ALERT; cleaned → OK | ✅ |
| T11 | engine actually uses `k` from `setK()` / `k()` getter | ✅ |
| T12 | no valid reading for 10 min → stale window forgets the alert; fresh 5-min confirmation required again | ✅ |
| T12b | short cloud spell (5 min) → alert **kept** | ✅ |
| T13 | window gated to one sample / 20 s: 15 instant reads = 1 sample; 15 reads at 20 s = full window + ALERT after 5 min | ✅ |
| T14 | `reset()` empties window and zeroes the mean | ✅ |

## 2. Mock cloud — unit tests (Python `unittest`)

**Result: `Ran 6 tests ... OK`**

| Test | What it proves | Result |
|---|---|---|
| POST form fields recorded | `/update` accepts form data, all 7 fields mapped | ✅ |
| GET query-style upload | firmware's `?field1=...&field7=1` style works (ThingSpeak-compatible) | ✅ |
| history appends in order | readings keep chronological order | ✅ |
| timestamp present | every record has `ts` | ✅ |
| non-numeric field rejected | `field1=garbage` → HTTP 400, nothing stored | ✅ |
| index page serves dashboard | `/` → HTTP 200 with the dashboard | ✅ |

## 3. Mock cloud — live HTTP smoke test (real server, real curl)

**Result: passed**

```
GET /update?field1=6.01&...&field7=0     -> ok
GET /data                                -> [{"v":6.01,"i":486.0,"p":2.92,...,"state":0,...}]
GET /update?...field6=20&field7=1        -> ok
GET /                                    -> HTTP 200
```

## 4. Firmware — compile check (arduino-cli, ESP32)

**Result: PASSED** — `arduino-cli compile --fqbn esp32:esp32:esp32 firmware/solar_health_monitor`
→ exit 0 (arduino-cli 1.5.2, esp32:esp32 3.3.12, all 4 libraries resolved)

```
Sketch uses 1059072 bytes (80%) of program storage space. Maximum is 1310720 bytes.
Global variables use 50704 bytes (15%) of dynamic memory, leaving 276976 bytes for local variables.
```

Command line note: on this PC the build needs `cmd.exe` in PATH:
`export PATH="/c/Windows/System32:$PATH"` before running arduino-cli in git-bash.

## 5. Hardware tests (blocked — parts not purchased yet)

Run in this order when hardware arrives (checklist + expected values: `03_hardware.md` § 5–6,
`06_integration.md` § 2):

| # | Test | Expected |
|---|---|---|
| 1 | Blink + serial banner | one line every 5 s @ 115200 |
| 2 | `tools/i2c_scanner` | 0x23, 0x40, 0x41 |
| 3 | DS18B20 reading | matches a thermometer, never −127 |
| 4 | INA219 vs multimeter | ±2 % voltage, ±5 % current |
| 5 | serial `c` (both panels clean, strong sun) | `k` 0.95–1.05, survives reboot |
| 6 | serial `t` test mode | soiling 20 % → after 5 min `state=1`, dashboard shows spike |
| 7 | `[cloud] ok` cadence | every 20 s while Wi-Fi up |
| 8 | dust test (flour on test panel) | alert after ~5 min → wipe → clears < 8 % |
| 9 | 24 h soak | zero resets, history continuous |

## 6. Bugs found & fixed during testing (TDD record)

| # | Phase | Issue | Fix | Re-test |
|---|---|---|---|---|
| 1 | red step | `soiling_logic.h` did not exist yet (expected TDD red state) | implemented the core header | → green |
| 2 | test compile | test passed a temporary `raw(...)` directly to `update(Reading&)` → *"invalid initialization of non-const reference"* | fixed the **test** (bind the reading to a named variable first) | 40/40 pass |
| 3 | test authoring | placeholder assertion in T8 referenced a stale decision | replaced with the real check before running | 40/40 pass |
| 4 | firmware compile | `calibrate()` used `raw.iTestMa`/`raw.iRefMa` — struct `Raw` members are `iT`/`iR` | corrected the member names | → compile PASS |
| 5 | firmware compile | function `cli()` collided with `Arduino.h`'s `cli()` macro (disable interrupts) | renamed to `pollSerial()` | → compile PASS |

Bugs 4–5 were caught by the compile check itself (the failing compile = the reproducing
test); both were then fixed and the build re-run to green.
