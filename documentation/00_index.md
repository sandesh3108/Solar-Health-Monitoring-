# Documentation Index — Solar Health Monitoring System

One folder (`documentation/`), one document per part, numbered in reading order.

| # | Document | Answers | Status |
|---|----------|---------|--------|
| 00 | [Index](00_index.md) (this file) | Where is everything? | current |
| 01 | [Overview](01_overview.md) | What is this project and why? | current |
| 02 | [System Design](02_system_design.md) | How is the system structured? | current |
| 03 | [Hardware](03_hardware.md) | What to buy, wire, assemble? | spec ready — parts not purchased yet |
| 04 | [Software](04_software.md) | How does the code work, build, run? | v1 built |
| 05 | [Testing](05_testing.md) | What was tested and what passed? | v1 results included |
| 06 | [Integration](06_integration.md) | How do the parts work together? | current |
| 07 | [Build Log](07_build_log.md) | What was built, decided, fixed? | running log |

## Reference documents (full detail, in this folder)

- **Solar Panel Health and Dust Monitoring - Complete Project Details.md** — the full build guide: theory, 25+ diagrams, complete wiring, dashboard design, report material.
- **Solar Panel Health and Dust Monitoring - Project Summary (Overview).docx** — printable 3-page summary.

## Repository layout

```
solar helth monitoring/
├── firmware/solar_health_monitor/    # ESP32 sketch (folder name == .ino name)
│   ├── solar_health_monitor.ino      # glue: read sensors -> log -> upload
│   ├── soiling_logic.h               # TESTED core logic (shared with PC tests)
│   └── config.h                      # pins, timing, Wi-Fi/cloud settings
├── tests/
│   ├── test_soiling_logic.cpp        # 40 checks of the alert/soiling logic (g++)
│   └── test_mock_cloud.py            # 6 tests of the mock cloud (unittest)
├── mock_cloud/server.py              # local cloud + live dashboard (Flask)
├── tools/i2c_scanner/                # bus scanner for hardware bring-up
├── documentation/                    # this folder
└── README.md
```

## Status at a glance

| Piece | State |
|---|---|
| Core logic (soiling %, window, alert FSM) | ✅ built, **40/40 unit checks pass** |
| Mock cloud + dashboard | ✅ built, **6/6 tests pass** + live HTTP smoke test pass |
| Firmware v1 (sensors, CLI, Wi-Fi upload) | ✅ written, **compile check PASS** for `esp32:esp32:esp32` (80 % flash / 15 % RAM) |
| Real hardware (ESP32 + panels) | ⏳ not purchased yet — flash & on-site tests pending |
