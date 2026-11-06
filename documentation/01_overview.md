# 01 — Project Overview

**Smart Solar Panel Health & Dust Monitoring System with Wireless Telemetry**
IoT / Embedded · ESP32 · Final-year project · Budget ≈ Rs 3,000-4,500

## Problem

Dust, pollen and bird droppings ("soiling") silently cut solar output by **5-30 %**.
Owners either clean too late (energy lost) or on a blind schedule (water/labour wasted),
because output also varies with clouds — a drop always looks "normal".

## Solution (the control experiment)

Two identical panels side by side:

- **test panel** — left uncleaned
- **reference panel** — kept spotless

Both see the same sun and temperature, so **any difference between them is caused by
dust alone**. That difference, as a percentage, is the **Soiling Index**:

```
Soiling % = (1 - (I_test / I_ref) / k) × 100        k = calibration ratio (both clean)
```

A light sensor (BH1750) gates the calculation so cloudy/night readings are ignored —
no false alerts. When the loss is real and sustained (> 15 % for 5 minutes) the system
raises **"Cleaning Required"** locally and on the cloud dashboard; it clears below 8 %.

## Objectives (v1 scope)

1. Measure V, I of both panels and compute power (P = V × I)
2. Measure light (lux) and panel temperature
3. Compute the soiling index with a stored calibration constant `k`
4. Alert logic with 5-minute rolling average + hysteresis (15 % / 8 %)
5. Fault detection (reference producing, test panel dead) and overheat warning (> 65 °C)
6. Wi-Fi upload of 7 values every 20 s to a cloud endpoint + serial console

## Not in v1 (backlog)

OLED display · buzzer/LED · relay/pump auto-cleaning · Blynk/Firebase · LoRa/GSM · ML

## Key numbers

| Item | Value |
|---|---|
| Read cadence | every 5 s (8 samples averaged) |
| Upload cadence | every 20 s (ThingSpeak-compatible ≥ 15 s rule) |
| Alert ON / OFF | mean > 15 % for 5 min / mean < 8 % |
| Light gate | lux > 20 000 and I_ref > 5 mA |
| Stale-data reset | after 10 min without a valid reading |
| I2C addresses | 0x40 test, 0x41 ref, 0x23 light (+0x3C OLED later) |

Full theory, diagrams and report material: see *Complete Project Details.md* in this folder.
