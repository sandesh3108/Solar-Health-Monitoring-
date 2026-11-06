# 07 — Build Log

Chronological record: decisions, steps, bugs, results. Newest entries appended.

## 2026-11-06 — v1 build (software first)

### Decisions taken with the user before coding

| # | Decision | Choice |
|---|---|---|
| D1 | Test depth | Unit tests **+** firmware compile check (arduino-cli) |
| D2 | Cloud | Local **mock cloud** first (ThingSpeak-compatible API) |
| D3 | Hardware | **None purchased yet** → flash step comes later |
| D4 | Scope | "keep it simple" → v1 = sensors + logic + serial + upload; **deferred**: OLED, buzzer/LED, relay |
| D5 | Docs | One `documentation/` folder, one markdown file per part; existing reference docs moved in |
| D6 | Repo | Push to `github.com/sandesh3108/Solar-Health-Monitoring-` when complete |

Assumptions listed before coding: spec = the existing detailed document (thresholds
15 %/8 %, lux 20 000, 20 s upload, addresses 0x40/0x41/0x23); ESP32 + Arduino framework;
core logic portable (no Arduino headers) so tests and firmware share one source; plain
`g++` assert tests; credentials placeholders (I cannot create accounts).

### Structure created

`firmware/solar_health_monitor/` (`.ino` + `soiling_logic.h` + `config.h`),
`tests/`, `mock_cloud/`, `tools/i2c_scanner/`, `documentation/`.
Note: core headers live **inside the sketch folder** (Arduino IDE compiles only files
inside the sketch folder); the PC tests `#include` them from there — one source of truth.

### TDD sequence (test first, always)

| Step | Action | Result |
|---|---|---|
| 1 | Wrote `tests/test_soiling_logic.cpp` (cases T1-T14, 40 checks) | **RED** — `fatal error: soiling_logic.h: No such file` |
| 2 | Wrote `soiling_logic.h` + `config.h` | — |
| 3 | Compiled | **RED (test bug)** — `invalid initialization of non-const reference ... from an rvalue` (a test passed `raw(...)` temporary directly to `update()`) |
| 4 | Fixed the *test* (bound the reading to a named variable first) | **GREEN — 40 checks, 0 failures** |
| 5 | Wrote `tests/test_mock_cloud.py` (6 tests) | **RED** — `server.py` missing |
| 6 | Wrote `mock_cloud/server.py` | **GREEN — 6/6 tests OK** |
| 7 | Wrote `firmware/solar_health_monitor/solar_health_monitor.ino` | compile **RED** — 2 bugs: `raw.iTestMa/iRefMa` (members are `iT`/`iR`) and `cli()` colliding with `Arduino.h`'s `cli()` macro |
| 8 | Fixed both (renamed `cli()` → `pollSerial()`, corrected member names) | compile **GREEN** — exit 0, 80 % flash / 15 % RAM |
| 9 | arduino-cli 1.5.2 + esp32:esp32 3.3.12 + 4 libraries installed for the check | `INSTALL_DONE`, all libraries resolved |
| 10 | Live mock-cloud smoke test (real HTTP via curl) | upload → `ok`, `/data` JSON correct, dashboard → HTTP 200 |

Also fixed during test authoring: removed a placeholder assertion in T8 (it referenced a
stale decision) — replaced with the real check before running.

### Environment notes

- Host: Windows, `g++` (MinGW), Python 3.13 + Flask, git with identity `sandesh3108`.
- No `gh` CLI → push done with plain `git`.
- The summary `.docx` was locked by another program → copied into `documentation/`
  (root duplicate to be removed when the file is closed).

### Backlog (v2, not started)

OLED UI · buzzer/LED · relay/pump · real ThingSpeak/Blynk switch · daily Wh ·
hardware flash & on-site test plan execution.
