# 02 — System Design

## 1. Big picture (one repeating loop)

```
 SUN → panels (test + reference) → INA219 ×2 → I2C → ESP32
        [average → P → soiling % → alert FSM] → serial log (every 5 s)
                                                  → Wi-Fi → mock cloud (every 20 s)
                                                     → dashboard / later: ThingSpeak
                                                   (v2: OLED + buzzer + relay)
```

Each layer fails softly: no Wi-Fi → measuring continues + serial log; no sun → readings
not evaluated; sensor missing → error printed, node keeps running.

## 2. Layers (what crosses between them)

| Layer | Hardware side | Software side | Data passed on |
|---|---|---|---|
| Sensing | panels, shunts, photodiode, temp junction | — | physical V/I/light/heat |
| Acquisition | I2C (GPIO21/22), 1-Wire (GPIO4) | sensor drivers + 8-sample average | V, mA, lx, °C |
| Processing | ESP32 CPU | `soiling_logic.h`: gate → soiling % → rolling window | soil %, validity, 5-min mean |
| Decision | ESP32 CPU | `AlertEngine`: fault > overheat > soiling (hysteresis) | `state` (0-3), `alarm` |
| Local output | USB serial (v1) | `printStatus()` every read | human-readable status line |
| Cloud | Wi-Fi radio + router | `upload()` HTTP GET of 7 fields | dashboard record + history |

## 3. Alert state machine

```
            full window & mean>15%            mean<8%
  ┌──────┐ ─────────────────────────▶ ┌────────┐ ──────────▶ back to OK
  │  OK  │                            │ ALERT │  (hysteresis: 8..15% HOLDS state)
  └──────┘ ◀───────────────────────── └────────┘
      ▲        mean<8% (after cleaning)
      │
 FAULT (I_ref>50 mA & I_test<5 mA) and OVERHEAT (T>65 °C) take priority.
 Skipped entirely while lux ≤ 20 000 ("LOW LIGHT").
 Stale >10 min → window forgotten → fresh 5-min confirmation needed again.
```

State codes uploaded as `field7`: 0 OK · 1 CLEANING · 2 FAULT · 3 OVERHEAT.

## 4. Timing of one cycle

| When | What happens | Cost |
|---|---|---|
| every 5 s | read 2×INA219 (8 samples each), lux, °C → gate → soiling → FSM → serial line | ≈ 0.3 s |
| every 20 s | Wi-Fi check + HTTP GET of 7 fields to `CLOUD_URL` | 1-3 s (3 s timeout) |
| between | serial console commands (`c/t/p/x/r`), Wi-Fi retry after 60 s | — |

Scheduling is `millis()`-based (no blocking `delay()` in `loop()`), so a dead network
cannot stall sensing.

## 5. Interface contract (firmware ⇄ cloud)

| Field | Meaning | Unit | Type |
|---|---|---|---|
| field1 | test-panel voltage | V | float |
| field2 | test-panel current | mA | float |
| field3 | test-panel power | W | float |
| field4 | light | lx | float |
| field5 | panel temperature | °C | float |
| field6 | soiling (5-min rolling mean) | % | float |
| field7 | alert state | 0-3 | int |

Transport: HTTP GET `CLOUD_URL?field1=...&field7=...` (ThingSpeak-compatible shape —
switching to ThingSpeak later is a one-line config change).

## 6. Design decisions (why)

| Decision | Why |
|---|---|
| Reference-panel method | measures the *real* loss; cancels weather/temperature automatically |
| Core logic in a header shared by tests + firmware | tested behaviour = deployed behaviour, no drift |
| 5-min window gated to one sample / 20 s | "sustained" alert without counting fast reads |
| Hysteresis 15 %/8 % | one clean transition, no flicker |
| Mock cloud first | fully testable today, no account/credentials needed |
| v2 = OLED/buzzer/relay | keep v1 minimal and easy to understand |
