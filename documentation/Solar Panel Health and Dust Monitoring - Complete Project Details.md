# Smart Solar Panel Health & Dust Monitoring System with Wireless Telemetry

## Complete Project Details & Build Guide

> **One-line summary:** An ESP32-based IoT node that compares a dusty solar panel against a clean reference panel, works out how much power the dust is stealing, and warns you (locally and on your phone) when cleaning is actually worth doing.

---

### Document Control

| Field | Value |
| --- | --- |
| Document | Complete Project Details & Build Guide, v2.0 |
| Based on | Project brief: "Smart Solar Panel Health & Dust Monitoring System with Wireless Telemetry" (v1.0) |
| Category | IoT / Embedded Systems / Renewable Energy |
| Target level | Final-year B.Tech / Diploma prototype (also usable as a freelance client project) |
| Controller | ESP32 DevKit (Wi-Fi, 2.4 GHz) |
| Core technique | Reference-panel soiling index with light-gated validity |
| Cloud options | ThingSpeak (easiest), Blynk (best mobile app), Firebase (custom page) |
| Estimated cost | Rs 2,000-3,000 basic · Rs 3,000-4,500 recommended college version · Rs 4,500-7,000 with auto-cleaning |
| Estimated build time | ~3 weeks part-time (plan in Part V) |
| Status | **Ready to build — this document contains the full design, wiring, code, dashboard and test plan. No further research needed.** |

---

### How To Use This Document

| If you want to... | Go to |
| --- | --- |
| Understand the idea in 5 minutes | Part 0 (Overview + Theory) |
| See the big design | Part I (System Design) |
| Buy parts | Part II § 2.2 (Bill of Materials) |
| Wire the circuit | Part II § 2.7-2.8 (Wiring) |
| Build the frame and enclosure | Part II § 2.11-2.13 |
| Set up the IDE and libraries | Part III § 3.2 |
| Understand / write the firmware | Part III § 3.4-3.9 |
| Set up dashboard and phone alerts | Part III § 3.12-3.13 |
| See expected graphs and sample data | Part IV |
| Plan the schedule and report | Part V |
| **Understand how hardware + software combine and work together** | **Part VI (last part)** |
| Revise before a viva | Appendix F |

---

## Table of Contents

**Part 0 — Project Foundation**
- 0.1 Abstract · 0.2 Problem · 0.3 Solution in plain words · 0.4 Objectives · 0.5 Scope · 0.6 Key features
- 0.7 Theory & working principles (solar cell basics, soiling science, why a reference panel, the math, sensor physics)

**Part I — System Design**
- 1.1 System context · 1.2 Block diagram · 1.3 Subsystems · 1.4 Deployment topology · 1.5 Software layer stack · 1.6 I2C bus topology · 1.7 Power distribution · 1.8 Data flow · 1.9 Design decisions (why this part, not that one)

**Part II — Hardware Part**
- 2.1 Overview · 2.2 Bill of Materials · 2.3 Tools · 2.4 Component deep-dive · 2.5 ESP32 pin map · 2.6 I2C addresses · 2.7 Master wiring table · 2.8 Connection diagrams · 2.9 Load resistor calculation · 2.10 Power budget · 2.11 Mechanical design · 2.12 Enclosure & weatherproofing · 2.13 Assembly procedure · 2.14 Hardware test & acceptance · 2.15 Hardware troubleshooting

**Part III — Software Part**
- 3.1 Goals · 3.2 Toolchain setup · 3.3 Project structure · 3.4 Software architecture · 3.5 Program flowcharts + alert state machine · 3.6 Module inventory · 3.7 Algorithms in detail · 3.8 Configuration constants · 3.9 Complete firmware code · 3.10 OLED UI · 3.11 Serial console & logs · 3.12 Cloud setup · 3.13 Dashboard design · 3.14 Test mode & software tests · 3.15 Software acceptance · 3.16 Software troubleshooting

**Part IV — Figures, Charts & Sample Data**
- 4.1 Clean vs dusty power chart · 4.2 One-week soiling chart · 4.3 Sunlight over a day · 4.4 Alert timeline · 4.5 Sample measurement table · 4.6 Blank data log template · 4.7 Which graphs your report needs

**Part V — Workflow & Project Plan**
- 5.1 End-to-end workflow · 5.2 Gantt chart · 5.3 Stage tasks with "done when" · 5.4 Team roles · 5.5 Deliverables checklist · 5.6 Report & PPT outline · 5.7 Risk register · 5.8 Operating & maintenance procedure · 5.9 Future scope

**Part VI — Integration: How Hardware and Software Combine**
- 6.1 The one-loop picture · 6.2 Layer-by-layer interface table · 6.3 Interface contract · 6.4 Bring-up order · 6.5 One measurement cycle (timing) · 6.6 Worked example: dust over a week · 6.7 Common integration problems · 6.8 Final integration checklist · 6.9 60-second plain-English explanation · 6.10 Quick-start card

**Appendices**
- A Formulas & calculations · B I2C scanner sketch · C Log/CSV format · D Glossary · E References & datasheets · F Viva questions with answers · G Freelance pricing estimates · H What changed from the original brief

---

# Part 0 — Project Foundation

## 0.1 Abstract

Dust, pollen and bird droppings ("soiling") silently reduce a solar panel's output — typically **5-30 %**, and much more in dry, dusty or roadside locations. Most small installations either never clean, or clean on a fixed schedule, wasting water and labour or cleaning too late.

This project builds a low-cost IoT monitoring node that continuously measures a **test (deliberately uncleaned) panel** and an identical **clean reference panel** under the same sunlight, computes a **Soiling Index (%)**, filters out unreliable readings using a light sensor, and reports live and historical data over Wi-Fi to a cloud dashboard. When the loss is real and sustained, it raises a **"Cleaning Required"** alert on an OLED display, a buzzer, an LED, the dashboard and the user's phone. An optional relay-driven pump can perform automatic cleaning.

Everything is built from commonly available modules around one ESP32, at a prototype cost of roughly **Rs 3,000-4,500**.

## 0.2 Problem Statement

| Aspect | Detail |
| --- | --- |
| What happens | A layer of dust absorbs and scatters sunlight before it reaches the cells |
| How much is lost | 5-30 % normally; up to 40 %+ after dust storms or long dry spells |
| Why it goes unnoticed | Output varies with clouds anyway, so a drop looks "normal" |
| Current behaviour | Blind fixed schedules → either too late (energy lost) or too early (water + labour wasted) |
| Who cares | Rooftop owners, solar farms, installer/O&M companies, colleges doing green-energy projects |
| What's missing | A cheap, automatic, **weather-aware** signal that says: "Today, cleaning will gain you X %" |

## 0.3 Solution in Plain Words

> **Analogy: a control experiment.** Put two identical plants side by side, water one and not the other. Any difference in growth is caused by the water — not by sunlight, soil or rain, because both plants got the same conditions.
>
> We do the same with two solar panels: keep one clean, let one get dusty. Both see identical sun and temperature, so the **difference between them is caused by dust alone**. That difference, as a percentage, is the Soiling Index.

**Why not just use a dust sensor?** Optical dust sensors (e.g. GP2Y1010) measure dust **floating in the air**, not the layer **stuck on the panel**. Airborne dust is a weak proxy; the reference panel measures the actual effect that matters — lost current.

## 0.4 Objectives

**Primary (must do)**
1. Measure panel voltage and current and compute power, P = V × I.
2. Measure sunlight intensity (lux) and panel/ambient temperature.
3. Estimate soiling loss using a clean reference panel and a calibration constant.
4. Detect abnormal output drops **after** correcting for light level.
5. Transmit readings wirelessly over Wi-Fi every 15-20 s.
6. Show live and historical data on a dashboard.
7. Generate a cleaning/maintenance alert (buzzer + dashboard + phone notification).

**Secondary (nice to have)**
8. Optional: trigger an automatic cleaning pump through a relay.
9. Log daily energy (Wh) and cleaning events for a "gain after cleaning" proof chart.
10. Fault detection (panel disconnected / shaded / damaged) and overheating warning.

## 0.5 Scope

| In scope (this build) | Out of scope (future / not needed for prototype) |
| --- | --- |
| Two small panels (test + reference) | Full-size 400 W array or string monitoring |
| Single node, single dashboard | Multi-node farm-wide SCADA |
| Wi-Fi telemetry within home/college range | LoRa / GSM / satellite links |
| Soiling % + alert + temperature warning | Full I-V curve tracing, MPPT analysis |
| Optional single-pump auto clean | Robotic/ wiper cleaning systems |
| Indirect soiling **estimate** (not lab-grade) | Certified pyranometer-grade measurement |

## 0.6 Key Features

**Must-have**
- Dual-channel V/I measurement (test vs reference)
- Power computation and rolling averages
- Soiling Index % with calibration constant `k`
- Light-threshold validity gate (no false alarms at night/cloud)
- Alert engine with hysteresis (15 % trigger, 8 % clear) + 5-minute confirmation
- Local 128×64 OLED status + buzzer + LED
- Wi-Fi upload every 20 s with auto-reconnect
- Cloud charts, status tile and phone/e-mail notification
- Calibration routine stored in flash (survives reboot)

**Optional**
- Relay + pump automatic cleaning
- Test mode with fake sensor values (for demo without sun)
- Daily energy (Wh) accumulation and cleaning-event history

---

## 0.7 Theory & Working Principles

### 0.7.1 Solar panel basics (the 60-second version)

A panel's current-voltage (I-V) curve has three numbers you need:

```
  I (A)
   ^
 Isc├━━━━━━━━━━━━╮                     ● = Maximum Power Point (MPP)
    │             ╰━╮                   where P = Vmp × Imp is maximum
    │      MPP ●    ╰━╮
    │    (Vmp,Imp)     ╰━╮
    │                    ╰━╮
    │                      ╰━━●
    └───────────┬───────────────▶ V (V)
              Vmp             Voc

 Isc = short-circuit current (sunlight drives current ≈ proportional to light)
 Voc = open-circuit voltage   (rises with light, falls with temperature)
 Vmp / Imp = voltage & current at the maximum-power operating point
```

| Symbol | Meaning | Why we care here |
| --- | --- | --- |
| Isc | Current with terminals shorted | Current ∝ sunlight → the basis of our dust comparison |
| Voc | Voltage with nothing connected | Must stay **below 26 V** (INA219 limit) |
| Vmp, Imp | Point of maximum power | Used to pick the load resistor |
| Pmax | Vmp × Imp | Panel rating, e.g. 6 V × 0.5 A = 3 W |
| Temperature coefficient | Power falls ≈ **0.4 % per °C above 25 °C** | Why we monitor temperature |

**Key physical fact used by this project:** at a fixed load and fixed sunlight, **current is very nearly proportional to sunlight**. So `I_test / I_ref` is a direct measure of how much sunlight the test panel is actually receiving *compared with* the clean one.

### 0.7.2 What is soiling?

| Type | Cause | Effect | Removable by |
| --- | --- | --- | --- |
| Dust/soil layer | Wind, dry climate, construction, roads | 5-30 % loss (broad, uniform) | Rain, washing |
| Bird droppings | Birds | Local shading, can damage cells (hot-spot) | Manual cleaning |
| Pollen / insect residue | Season | 3-10 %, sticky film | Washing with water |
| Film / hard-water stains | Irrigation spray (fogging) | Permanent if untreated | Special cleaning |
| Shading | Nearby object growth | Can look like soiling | Trimming |

Losses compound: dust + hot weather + partial shade can push output far below the panel's rating.

### 0.7.3 Ways to measure soiling — and why we chose the reference panel

| Method | How it works | Pros | Cons | Cost |
| --- | --- | --- | --- | --- |
| Optical dust sensor (GP2Y1010) | Measures airborne particle density | Cheap, simple | Measures **air**, not the panel layer; weak correlation | Low |
| LDR / photoresistor | Resistance changes with light | Very cheap | Non-linear, drifts, no absolute scale | Very low |
| **Reference panel / reference cell** | Clean panel vs test panel, same sun | Directly measures the real loss; self-correcting for weather | Needs a second panel and discipline (keep it clean!) | Low |
| Soiling station (commercial) | Two mounted cells with automatic wiping | Accurate, standard in solar farms | Expensive, complex | High |
| I-V curve tracer | Full curve comparison | Most complete data | Expensive, needs disconnecting the array | High |
| Camera + image analysis | Photo of panel, ML detects dirt | Novel, no second panel | Needs light control, computing, training data | Medium |
| Power-model vs actual | Predict expected power from weather API, compare | No extra hardware | Needs accurate irradiance/weather data, model tuning | Medium |

**Choice: reference-panel method.** It is cheap, robust, directly measures what we care about (lost current), and naturally cancels temperature and cloud effects because both panels experience them equally.

### 0.7.4 The mathematics

**1. Calibration constant `k`** — measured once, with *both* panels spotless in strong sunlight:

```
        I_test (clean at that moment)
  k  =  ─────────────────────────────
        I_ref (clean at that moment)
```

`k` absorbs small manufacturing differences (one panel may be 4 % weaker out of the factory). Expect **k ≈ 0.95 - 1.05**.

**2. Soiling Index** — computed continuously:

```
                    ( I_test / I_ref )
  Soiling (%) = ( 1 - ───────────────── ) × 100
                         k
```

- `Soiling = 0 %` → test panel as good as the clean reference.
- `Soiling = 20 %` → the dust is costing you one-fifth of your output.
- Negative values (e.g. −3 %) are just measurement noise; clamp/plot as ~0.

**3. Worked example** — `k = 1.00`, `I_test = 400 mA`, `I_ref = 500 mA`, `lux = 42 000`:

```
  Soiling = (1 - (400/500)/1.00) × 100 = (1 - 0.80) × 100 = 20 %   → ALERT
```

**4. Validity gate** — only trust the number in good light:

```
  valid  =  ( lux > 20 000 lx )   AND   ( I_ref > 5 mA )
  if (!valid) → do NOT compute/act; display "LOW LIGHT"
```

**5. Rolling average** (noise filter + 5-minute confirmation):

```
  avg_soiling = mean of the last 15 valid samples   (15 × 20 s ≈ 5 min)

  if (window full AND avg_soiling > 15 %) → ALERT ON
  if (avg_soiling <  8 %)                 → ALERT OFF      (hysteresis)
```

**6. Hysteresis** — why two thresholds:

```
  Soiling %
   20 │                    ┌────────── ALERT ON (buzzer, LED, push)
   15 │ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─│─ ─ ─ ─ ─ trigger level
      │                    │
    8 │ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─│─ ─ ─ ─ ─ clear level
      │   ┌────────────────┘
    0 │───┘
      └──────────────────────────────▶ time

  A single threshold at 15 % would make the buzzer flicker on/off as the
  reading oscillates 14.7 ↔ 15.3. Two thresholds (15 % on, 8 % off) = one
  clean transition.
```

**7. Temperature correction**

```
  P_loss_temp ≈ P_rated × 0.004 × (T_panel − 25 °C)
```

Because the two panels sit side by side, at nearly the same temperature, **the ratio method cancels this automatically**. The DS18B20 reading is therefore used mainly for:
- an **overheating warning** (> 65 °C, e.g. back-sheet delamination, poor ventilation),
- dashboard context, and
- post-hoc analysis of your test data.

### 0.7.5 How each sensor works (so you can explain it in a viva)

| Sensor | Principle | What it gives us | Key limits |
| --- | --- | --- | --- |
| **INA219** | Measures voltage drop across a precision 0.1 Ω shunt (Ohm's law: I = V_shunt / 0.1), amplifies it with an instrumentation amp, converts with a 12-bit ADC, and reports bus voltage + current over I2C | V (0-26 V) and I (up to ~3.2 A) | Bus ≤ 26 V; shunt polarity matters (backwards = negative current) |
| **BH1750** | Photodiode + integrator ADC; counts are converted to lux (resolution 1 lx, range 1-65 535 lx) | Ambient light level for the validity gate | Saturates near 65 k lx in full sun — that's fine, we only need "> 20 000" |
| **DS18B20** | Band-gap temperature reference digitised inside the chip; communicates over 1-Wire | Panel/back temperature ±0.5 °C | Needs a 4.7 kΩ pull-up; −127 °C = wiring error |
| **SSD1306 OLED** | Self-emitting organic LED matrix, 128×64 pixels, I2C | Local display that works with no Wi-Fi | 3.3 V only; default address 0x3C |
| **ESP32** | Dual-core 240 MHz MCU with 2.4 GHz Wi-Fi + 3.3 V GPIO, I2C, 1-Wire, ADC | Reads sensors, computes, decides, uploads | 3.3 V logic; **Wi-Fi is 2.4 GHz only**; avoid flash pins GPIO 6-11 |

### 0.7.6 End-to-end physical story (one sentence per stage)

1. **Sun** photons hit both panels → both push current through their identical load resistors.
2. **Shunts** create a tiny voltage drop proportional to each current → **INA219s** digitise it.
3. **BH1750** reports the sunlight level; **DS18B20** reports the panel temperature.
4. **ESP32** reads all of them over two digital buses (I2C + 1-Wire), averages them, and computes power, the soiling index and the alert state.
5. **OLED + buzzer + LED** show/announce the result locally in real time.
6. **Wi-Fi** uploads seven values to the **cloud** every 20 s.
7. **Dashboard + notification** turn those values into graphs and a phone alert for the user.
8. *(Optional)* the ESP32 switches a **relay** to run the cleaning pump.

---

---

# Part I — System Design

## 1.1 System Context (what is inside the system, what is outside)

```
                          ┌────────────────────────────┐
   Sunlight ─────────────▶│                            │
                          │   SOLAR HEALTH MONITORING  │──── HTTP/JSON ───▶ ┌──────────────┐
   User (views/cleans) ◀──│          NODE              │      (Wi-Fi)       │   CLOUD      │──▶ Web/Mobile
        ▲                 │  2 panels + sensors +      │                    │ ThingSpeak / │    Dashboard
        │                 │  ESP32 + relay (optional)  │◀─── commands ──────│ Blynk /      │    + Push
        │                 └────────────────────────────┘                    │ Firebase     │    Alerts
        │                       │           │                               └──────────────┘
        │                  5 V supply    local output
        │                  (USB adapter) (OLED, buzzer, LED)
        │                       │
        └───────────┐      ┌────┴────┐      ┌─────────────────┐
                    │      │ Router  │      │ Cleaning pump   │ (optional,
                    └──────│ (2.4 GHz)│     │ via relay       │  separate supply)
                           └─────────┘      └─────────────────┘
```

**Inside the system:** panels, sensors, ESP32, enclosure, firmware, cloud channel, dashboard.
**Outside (external entities):** sun/weather, mains power, Wi-Fi router, internet, the human user.

## 1.2 Master Block Diagram

```
   ┌────────────────┐  ┌────────────────┐
   │  TEST PANEL    │  │ REFERENCE PANEL│        (identical pair, same tilt & azimuth)
   │   (dusty)      │  │   (kept clean) │
   └───────┬────────┘  └───────┬────────┘
           │ +                  │ +
           ▼                    ▼
   ┌───────────────┐   ┌───────────────┐
   │  INA219  #1   │   │  INA219  #2   │      high-side series measurement
   │  addr 0x40    │   │  addr 0x41    │      V 0-26 V, I up to 3.2 A
   └───────┬───────┘   └───────┬───────┘
           │ V, I (mA)         │ V, I (mA)
           │                   │
   ┌───────┴───────┐   ┌───────┴───────┐
   │  R_load (test)│   │  R_load (ref) │      identical fixed loads
   └───────┬───────┘   └───────┬───────┘
           │ GND               │ GND
           └─────────┬─────────┘
                     │
      ┌──────────────┼───────────────────────────────┐
      │   I2C BUS (SDA = GPIO21, SCL = GPIO22)      │
      │   + 4.7 kΩ pull-ups to 3.3 V                │
      │              │                               │
      │   ┌──────────┴─────────┐  ┌──────────────┐   │
      │   │  BH1750   (0x23)   │  │ OLED SSD1306 │◄──┼── local display
      │   │  light (lux)       │  │   (0x3C)     │   │   (works offline)
      │   └────────────────────┘  └──────────────┘   │
      │                                              │
      │   ┌────────────────────┐                     │      ┌───────────────┐
      │   │ DS18B20 (GPIO4)    │─────────────────────┼─────▶│               │
      │   │ panel temperature  │  1-Wire + 4.7 kΩ    │      │               │
      │   └────────────────────┘                     │      │     ESP32     │
      │                                              │      │  • read sense │
      │             GPIO25 ──▶ buzzer + LED          │─────▶│  • avg/filter │
      │             GPIO26 ──▶ status LED            │      │  • P = V × I  │
      │             GPIO27 ──▶ relay (optional) ──▶ pump    │  • soiling %  │
      │                                              │      │  • alert FSM  │
      └──────────────────────────────────────────────┘      │  • OLED/Wi-Fi │
                                                            └──────┬────────┘
                                                                   │ Wi-Fi 2.4 GHz
                              ┌────────────────────────────────────┘
                              ▼
                     ┌─────────────────┐        ┌──────────────────────┐
                     │  Router / AP    │───────▶│  CLOUD               │
                     └─────────────────┘  internet│ ThingSpeak/Blynk/   │
                                                  │ Firebase            │
                                                  └─────────┬──────────┘
                                                            │
                                             ┌──────────────┴──────────────┐
                                             ▼                             ▼
                                     ┌──────────────┐              ┌──────────────┐
                                     │  Web dashboards│             │ Phone push / │
                                     │  charts, gauges│             │ e-mail alert │
                                     └──────────────┘              └──────────────┘
```

## 1.3 Subsystem Decomposition

| # | Subsystem | Hardware part | Software part | Output |
| --- | --- | --- | --- | --- |
| S1 | Energy harvesting measurement | 2 × panel, 2 × INA219, 2 × load resistors | INA219 drivers, averaging | V_test, I_test, V_ref, I_ref |
| S2 | Environmental sensing | BH1750, DS18B20 | Lux/temp drivers | lux, °C |
| S3 | Processing & decision | ESP32 CPU | Power math, soiling %, rolling avg, alert FSM | P, soiling %, state |
| S4 | Local HMI | OLED, buzzer, LED | Display/GPIO code | On-site readout + alarm |
| S5 | Communication | ESP32 Wi-Fi radio, router | Wi-Fi manager, HTTP uploader | 7 fields every 20 s |
| S6 | Cloud & user | Internet, cloud server, phone/PC | Dashboard widgets, React/Blynk notifications | Graphs, alerts |
| S7 | Actuation (optional) | Relay, pump, tubing, supply | Relay timing logic | Automatic cleaning |

## 1.4 Deployment / Network Topology

```
   [ RIG — outdoors ]              [ INDOORS ]              [ INTERNET / CLOUD ]

  ┌────────────────────┐         ┌──────────────┐         ┌─────────────────────┐
  │ Test panel (dusty) │         │  Wi-Fi Router│         │  ThingSpeak channel │
  │ Ref panel (clean)  │  Wi-Fi  │  2.4 GHz SSID│  HTTP   │  fields 1-7         │
  │ ESP32 + sensors    │────────▶│  + internet  │────────▶│  charts + React      │
  │ in enclosure       │  ~15-50 m│  (2.4 GHz!)  │  (TLS)  │                     │
  └────────────────────┘         └──────────────┘         └──────────┬──────────┘
                                                                     │
                                            ┌────────────────────────┼──────────────┐
                                            ▼                        ▼              ▼
                                     ┌────────────┐          ┌────────────┐  ┌────────────┐
                                     │ Phone app/ │          │ Laptop     │  │ E-mail     │
                                     │ push alert │          │ dashboard  │  │ alert      │
                                     └────────────┘          └────────────┘  └────────────┘
```

| Range consideration | Rule |
| --- | --- |
| ESP32 to router | Up to ~30-50 m outdoors (walls reduce it). Use an extender if weaker than about −70 dBm. |
| Band | **2.4 GHz only** — ESP32 cannot join a 5 GHz-only network. |
| Data rate | 7 numbers × every 20 s ≈ trivial (< 1 KB/s), works even on weak links. |
| Offline | Node keeps measuring and showing OLED; uploads resume when Wi-Fi returns. |

## 1.5 Software Layer Stack (design view)

```
┌─────────────────────────────────────────────────────────────────────┐
│ 6. PRESENTATION      OLED screens · dashboard widgets · notifications│
├─────────────────────────────────────────────────────────────────────┤
│ 5. APPLICATION       calibration · soiling calculation · alert FSM  │
├─────────────────────────────────────────────────────────────────────┤
│ 4. SERVICES          Wi-Fi manager · cloud uploader · scheduler     │
│                      (millis-based, non-blocking) · serial logging  │
├─────────────────────────────────────────────────────────────────────┤
│ 3. DRIVERS           INA219 ×2 · BH1750 · DS18B20 · SSD1306 · GPIO │
├─────────────────────────────────────────────────────────────────────┤
│ 2. ABSTRACTION       Wire (I2C) · OneWire · digitalWrite · Preferences│
├─────────────────────────────────────────────────────────────────────┤
│ 1. PLATFORM          ESP32 Arduino core (+ FreeRTOS underneath)     │
└─────────────────────────────────────────────────────────────────────┘
   Each layer only talks to the one below it → a fault can be traced to
   exactly one layer (this is why integration debugging becomes easy).
```

## 1.6 I2C Bus Topology

```
   ESP32                                                    +3.3 V rail
   ┌──────────┐                                                 │
   │ GPIO21   │── SDA ──┬──────────┬──────────┬──────────┐      │
   │ GPIO22   │── SCL ──┼──────────┼──────────┼──────────┼──┐   │
   │          │         │          │          │          │  │   │
   │ 3.3 V    │─────────┴──────────┴──────────┴──────────┘  │   │
   │ GND      │─── common ground rail ──────────────────────┼───┤
   └──────────┘         │          │          │          │  │   │
                        │          │          │          │  │   │
                   ┌────┴────┐ ┌───┴─────┐ ┌──┴──────┐ ┌─┴───────┐
                   │ INA219  │ │ INA219  │ │ BH1750  │ │ OLED    │
                   │ #1      │ │ #2      │ │         │ │SSD1306  │
                   │ 0x40    │ │ 0x41    │ │ 0x23    │ │ 0x3C    │
                   └─────────┘ └─────────┘ └─────────┘ └─────────┘
                        ▲           ▲
              A0→GND (default)  A0→VCC (bridge the A0 pad to VCC)

   • ONE pair of 4.7 kΩ pull-ups on the bus (many modules already have
     their own — if all are fitted, that's fine at short cable lengths).
   • Total bus capacitance limit 400 pF ≈ short breadboard runs are fine.
   • Separate bus? If you ever see instability, give the OLED its own
     bus — but for 4 devices on < 30 cm of wire, one bus is fine.
```

## 1.7 Power Distribution Tree

```
                        ┌──────────────────────────────────────────────────┐
   230 V AC ──▶ USB adapter ── 5 V / ≥1 A ──▶ ESP32 5V pin               │
                        │                   (on-board LDO)                │
                        │                            │ 3.3 V              │
                        │          ┌─────────────────┼──────────────┐     │
                        │          ▼                 ▼              ▼     │
                        │     INA219 #1,#2       BH1750, OLED    DS18B20  │
                        │     (VCC 3.3 V)        (VCC 3.3 V)   (VDD 3.3V)│
                        │                                                 │
   TEST PANEL ──▶ INA219 #1 ──▶ R_load  ──┐  (measurement loop only)     │
   REF  PANEL ──▶ INA219 #2 ──▶ R_load  ──┤  panels NEVER power the ESP32│
                                          └── both returns → common GND   │
                                                                              │
   (optional) separate 5/12 V supply ──▶ relay module ──▶ pump              │
              (common GND with ESP32, flyback diode across pump)            │
```

**Golden rules**
1. The ESP32 is powered from a **regulated 5 V ≥ 1 A** adapter (Wi-Fi causes current spikes).
2. The panels only drive their **load resistors**; they never power the electronics.
3. Everything shares **one common ground**, or the measurements will be meaningless/dangerous.
4. The optional pump gets its **own supply** so its inrush does not reset the ESP32.

## 1.8 Data Flow Diagram

```
  PHYSICAL            DIGITAL                 INFORMATION            USER
 ─────────           ─────────                ───────────            ────
 sunlight ─┐
           ├─▶ current(mA) ─┐
 dust ─────┘                │
                            ├─▶ average ─▶ P=V×I ─┐
 sunlight ─▶ lux ───────────┤                     ├─▶ soiling % ─▶ rolling avg ─▶ ALERT?
 temperature ─▶ °C ─────────┘                     │        ▲                        │
                                                  │        └── gated by lux         │
                                                  │                                 ▼
                              OLED/buzzer/LED ◀───┴────────────────────── HTTP POST every 20 s
                                                                            │
                                                                            ▼
                                                              cloud stores 7 fields
                                                                            │
                                                              ┌─────────────┴─────────────┐
                                                              ▼                           ▼
                                                        charts/gauges              notification
                                                        (history)                  (if state=ALERT)
```

| Stage | Data | Units | Rate |
| --- | --- | --- | --- |
| Sensor → ESP32 | raw registers | counts, mV | every read (8 samples / 100 ms) |
| ESP32 internal | engineering values | V, mA, W, lx, °C | every cycle (5 s read, 20 s upload) |
| ESP32 → cloud | 7 fields | V, mA, W, lx, °C, %, 0/1 | every 20 s |
| Cloud → user | charts + events | rendered graphs, push | live/continuous |

## 1.9 Design Decisions (why this, not that)

| Decision | Alternatives considered | Why chosen |
| --- | --- | --- |
| **ESP32** | Arduino Uno (no Wi-Fi), ESP8266 (1 ADC, fewer pins), Raspberry Pi (overkill, boots from SD, power hungry) | Built-in Wi-Fi, enough GPIO, dual I2C-capable, huge library support, cheap |
| **INA219** | ACS712 (analogue, noisy at small currents), raw ADC + shunt (calibration pain) | Digital, calibrated, I2C, measures V **and** I, 0-26 V / 3.2 A fits small panels |
| **Reference panel method** | Air-dust sensor, LDR, weather-API model | Directly measures actual loss; cancels weather and temperature; explainable in a viva |
| **BH1750 validity gate** | LDR, no gate at all | Digital, linear in lux, cheap; prevents night/cloud false alerts |
| **I2C shared bus** | One wire per device | 4 devices on 2 pins; addresses don't collide |
| **20 s upload interval** | 1 s streaming | ThingSpeak free tier needs ≥ 15 s; also plenty for a slowly changing quantity |
| **Hysteresis 15/8 %** | Single threshold | Prevents alert flicker; 15 % = clearly worth cleaning |
| **Fixed resistive load** | MPPT/charge controller/battery | Simple, cheap, repeatable, and identical on both panels = fair comparison |
| **ThingSpeak as default** | Blynk, Firebase, MQTT broker | Fastest to a working chart with 8 fields, no server to run; Blynk better for mobile UI |

---
---

# Part II — Hardware Part

> **Goal of this part:** a physically built, weather-protected rig where every sensor reads correctly and all four I2C addresses are visible.
> **Acceptance line:** *"Hardware is done when the I2C scan shows 0x23, 0x3C, 0x40, 0x41, every reading matches a multimeter/thermometer within a few percent, and the wiring survives being moved."*

## 2.1 Hardware Overview

The hardware is **six physical groups**:

1. **Energy group** — two identical small panels, each in series with an INA219 and an identical load resistor.
2. **Sense group** — BH1750 (light) + DS18B20 (temperature) attached to the panel.
3. **Brain** — ESP32 DevKit.
4. **Local feedback** — OLED display, buzzer, status LED.
5. **Power group** — 5 V/1 A USB adapter, distribution wires, common ground.
6. **Optional cleaning group** — relay module, mini pump, tubing, separate supply, enclosure/frame.

## 2.2 Bill of Materials (BOM)

| # | Component | Key specification to buy | Qty | Approx. cost (INR) | Notes |
| --- | --- | --- | --- | --- | --- |
| 1 | ESP32 DevKit | 30- or 38-pin, WROOM module | 1 | 430-500 | Buy the common "ESP32-WROOM-32 DevKit v1" |
| 2 | Solar panel (identical pair) | 6 V / 2-3 W, **Voc < 26 V** | 2 | 500-1,000 | Both from the **same batch/model** — critical |
| 3 | INA219 module | 0-26 V, 0.1 Ω shunt, I2C | 2 | 250-400 | Second one needs A0 bridged to VCC (0x41) |
| 4 | BH1750 module | GY-302, I2C, 1-65535 lx | 1 | 70-150 | ADDR low = 0x23 |
| 5 | DS18B20 | Waterproof stainless probe + cable | 1 | 80-150 | Plus **4.7 kΩ resistor** |
| 6 | OLED display | 0.96" SSD1306, 128×64, I2C | 1 | 120-250 | Address 0x3C |
| 7 | Buzzer | Active, 3-5 V | 1 | 20-50 | Add NPN transistor if > 12 mA |
| 8 | Status LED + resistor | 5 mm + 220-330 Ω | 1 set | 10-30 | GPIO26 indicator |
| 9 | Load resistors | **R = Vmp/Imp**, ≥ 5 W (wire-wound/cement) | 2 | 20-60 | e.g. 12 Ω 10 W; **same value both sides** |
| 10 | 4.7 kΩ resistors | ¼ W | 2 | 5 | DS18B20 pull-up; spare for I2C |
| 11 | Breadboard/perfboard + jumpers | M/F jumpers, 40-wire ribbon | 1 set | 150-300 | Perfboard for the final build |
| 12 | 5 V / 1 A (≥) USB adapter + cable | Regulated | 1 | 100-300 | Powers the ESP32 only |
| 13 | Enclosure | IP54-ish plastic box + cable glands | 1 | 200-500 | Add silica-gel sachet |
| 14 | Mounting frame | Aluminium/wood bar, L-brackets, screws | 1 | 200-400 | Fixed tilt, both panels identical |
| 15 | Thermal tape / adhesive pad | For DS18B20 on panel back | 1 | 50-100 | Aluminium tape works well |
| 16 | Wires | 22-24 AWG (panel lines), hook-up wire | 1 roll | 100-200 | **Same gauge & length both panels** |
| 17 | Multimeter (if you don't have one) | Basic DMM | 1 | 300-700 | Needed for calibration cross-checks |
| **Optional auto-cleaning** |||||
| 18 | Relay module | 1-channel, opto-isolated, 5 V | 1 | 80-150 | Flyback protection for the pump |
| 19 | Mini pump | 3-6 V diaphragm/brushless + tubing | 1 | 200-600 | Or a 5 V wiper motor |
| 20 | Separate supply for pump | 5-12 V adapter matching the pump | 1 | 100-200 | Keep common GND with ESP32 |
| 21 | 1N4007 diode | Flyback across pump | 1 | 5 | Across pump terminals |

**Budget summary**

| Version | Contents | Approx. total |
| --- | --- | --- |
| Basic prototype | Items 1-14 (minimal wiring) | Rs 2,000-3,000 |
| **Recommended college version** | Items 1-17, nice enclosure, good frame | **Rs 3,000-4,500** |
| Advanced (auto-cleaning) | + items 18-21 | Rs 4,500-7,000 |

> Prices are estimates — check current marketplace prices before ordering, and keep the bill for your report.

## 2.3 Tools & Consumables

| Tool | Used for |
| --- | --- |
| Multimeter (V, mA, Ω, continuity) | Verifying every sensor and polarity |
| Soldering iron + solder + flux | Perfboard assembly |
| Wire stripper / cutter | Wiring |
| Screwdriver set, small drill or hot glue | Enclosure and frame |
| Ruler/protector + marker | Equal tilt marking |
| PC with Arduino IDE (or PlatformIO) | Firmware upload |
| Breadboard | First bring-up before soldering |

## 2.4 Component Deep-Dive

### 2.4.1 ESP32 DevKit (the brain)

| Item | Detail |
| --- | --- |
| CPU | Xtensa dual-core 240 MHz, 520 KB SRAM |
| Wireless | Wi-Fi 802.11 b/g/n **2.4 GHz only** + Bluetooth |
| Logic level | **3.3 V — never connect 5 V to a GPIO** |
| Power | 5 V on VIN/USB → on-board regulator → 3.3 V |
| Pins used here | GPIO21 (SDA), 22 (SCL), 4 (1-Wire), 25 (buzzer), 26 (LED), 27 (relay) |
| **Pins to avoid** | GPIO **6-11** (connected to flash — board dies), GPIO 0/2/12/15 (boot/strap pins), GPIO 34-39 (input-only) |

```
   ESP32 DevKit (38-pin) — pins used in this project
                    ┌──────────────┐
   3V3  ────────────┤ 3V3       5V ├────────── 5 V from USB adapter
   GND  ────────────┤ GND      GND ├────────── common ground
   IO4  ────────────┤ IO4    (1W)  ├  DS18B20 data (+4.7 kΩ to 3V3)
   IO25 ────────────┤ IO25   OUT   ├  buzzer
   IO26 ────────────┤ IO26   OUT   ├  status LED (220 Ω)
   IO27 ────────────┤ IO27   OUT   ├  relay IN (optional)
   IO21 ────────────┤ IO21  SDA    ├  I2C data  (to all 4 I2C devices)
   IO22 ────────────┤ IO22  SCL    ├  I2C clock
                    └──────────────┘
```

### 2.4.2 INA219 (voltage + current, ×2)

| Item | Detail |
| --- | --- |
| Measures | Bus voltage 0-26 V, shunt voltage ±320 mV → current via 0.1 Ω shunt ⇒ up to ~3.2 A |
| Interface | I2C, addresses **0x40** (A0→GND), **0x41** (A0→VCC), 0x44, 0x45 |
| Power | 3 V-5 V VCC (we use 3.3 V) |
| Resolution | 10-bit current (with averaging in library), 12-bit voltage |
| Gotcha | **Shunt polarity matters.** Current flowing VIN+ → VIN− gives a positive reading; backwards gives negative. |

```
   Module terminals (typical green/blue board):
     [ VIN+ ]  [ VIN- ]  [ VCC ]  [ GND ]  [ SDA ]  [ SCL ]     (+ pads A0, trigger)

   Address selection pad "A0":  → GND = 0x40 (default)
                                 → VCC = 0x41   (solder a bridge/wire on the 2nd unit)
```

### 2.4.3 BH1750 (light, lux)

| Item | Detail |
| --- | --- |
| Range | 1 - 65 535 lx, resolution 1 lx, continuous high-resolution mode |
| Address | **0x23** (ADDR pin low) / 0x55 (ADDR high) |
| Conversion time | ~120-180 ms per reading — allow a short delay after `begin()` |
| Pins | VCC (3.3 V), GND, SDA, SCL, ADDR |
| Reality check | Full sunlight ≈ 100 000 lx **saturates** it near 65 k — perfectly fine, we only test `> 20 000` |

### 2.4.4 DS18B20 (temperature)

| Item | Detail |
| --- | --- |
| Accuracy | ±0.5 °C in −10…+85 °C range |
| Resolution | 9-12 bit (0.5-0.0625 °C); 12-bit conversion ≈ 750 ms |
| Wiring | 3 wires: VDD (3.3 V), GND, DATA (GPIO4) + **4.7 kΩ pull-up DATA→3.3 V** |
| Failure symptom | **−127 °C** = no pull-up / loose data wire |
| Mounting | Taped flat to the **back** of the test panel with thermal tape (measure bond, not air) |

```
      3.3 V ─────────┬──────────────┐
                     │ 4.7 kΩ       │ DS18B20
                     ├──────┬───────┤  ┌───────┐
                     │      │ DATA  ├──┤ DATA  │
                     │      │       │  │ VDD   ├── 3.3 V
                     │      └── GPIO4   │ GND   ├── GND
                     │                  └───────┘
```

### 2.4.5 OLED SSD1306 (local display)

| Item | Detail |
| --- | --- |
| Panel | 0.96", 128×64 px, white/blue, self-emitting (no backlight needed) |
| Address | **0x3C** (0x3D if soldered otherwise) |
| Pins | VCC (3.3 V), GND, SDA, SCL |
| Value | Local readout **works even when Wi-Fi/cloud is down** — essential for demos |

### 2.4.6 Buzzer, LED, relay

| Item | Wiring | Note |
| --- | --- | --- |
| Active buzzer | GPIO25 → (1 kΩ → base of 2N2222/BC547) → buzzer+ ; buzzer− → 5 V; emitter → GND | Use direct GPIO drive only if the buzzer draws ≤ ~12 mA; a transistor is safest |
| Status LED | GPIO26 → 220-330 Ω → LED+ ; LED− → GND | Current ≈ 5-10 mA, safe for a GPIO |
| Relay module | GPIO27 → IN; VCC → 5 V; GND → GND; COM/NO → pump supply | **Separate supply for the pump** + **1N4007 flyback diode** across pump terminals. If the module has a JD-VCC jumper, powering the relay from a separate supply isolates it from the ESP32 |

### 2.4.7 Solar panels (test + reference)

| Item | Detail |
| --- | --- |
| Type | Small 6 V / 2-3 W polycrystalline or monocrystalline, **identical model from the same batch** |
| Typical label | Voc ≈ 7.2 V, Vmp = 6 V, Isc ≈ 0.55 A, Imp = 0.5 A, Pmax = 3 W |
| Hard limit | **Voc must be < 26 V** (INA219) |
| Handling | Test panel: never clean after calibration day. Reference panel: wipe weekly with a dry cloth (log the date!) |

### 2.4.8 Load resistors (the panel's "work")

Panels must be **loaded** to produce current — an open-circulating panel gives ~0 current and no comparison. See § 2.9 to compute the value.

## 2.5 ESP32 Pin Mapping (master table)

| Function | ESP32 pin | Type | Notes |
| --- | --- | --- | --- |
| I2C SDA | **GPIO21** | Digital (I2C) | Shared: INA219 ×2, BH1750, OLED |
| I2C SCL | **GPIO22** | Digital (I2C) | Shared by all I2C devices |
| DS18B20 data | **GPIO4** | 1-Wire | 4.7 kΩ pull-up to 3.3 V |
| Buzzer | **GPIO25** | PWM/Digital out | Transistor driver if needed |
| Status LED | **GPIO26** | Digital out | Series 220-330 Ω |
| Relay IN (optional) | **GPIO27** | Digital out | Opto-isolated module |
| Unused / free | GPIO13, 14, 18, 19, 23, 32, 33 | — | Reserved for future sensors/pump feedback |
| **Do not use** | 6-11 (flash), 0/2/12/15 (strap), 34-39 (input only) | — | Would break boot or cannot output |

**Code-side:** `Wire.begin(21, 22);` `OneWire oneWire(4);` `pinMode(25/26/27, OUTPUT);`

## 2.6 I2C Address Map

| Device | Address | How it is set | Verify with scanner |
| --- | --- | --- | --- |
| INA219 #1 (test panel) | **0x40** | A0 pad → GND (default) | ✔ |
| INA219 #2 (reference) | **0x41** | Bridge A0 pad → VCC on the module | ✔ |
| BH1750 | **0x23** | ADDR pin low (default) | ✔ |
| OLED SSD1306 | **0x3C** | Module default | ✔ |

> If two devices report the same address, the bus will show garbage or lock up — always run the I2C scanner (Appendix B) after wiring.

## 2.7 Master Wiring Table (every single wire)

| From | To | Wire |
| --- | --- | --- |
| USB adapter 5 V | ESP32 5V (or USB port) | Power cable |
| ESP32 GND | Breadboard GND rail | Black |
| Breadboard GND rail | Test panel (−) | Black |
| Breadboard GND rail | Reference panel (−) | Black |
| ESP32 3V3 | Breadboard 3.3 V rail | Red |
| 3.3 V rail | INA219 #1 VCC | Red |
| 3.3 V rail | INA219 #2 VCC | Red |
| 3.3 V rail | BH1750 VCC | Red |
| 3.3 V rail | OLED VCC | Red |
| 3.3 V rail | DS18B20 VDD | Red |
| GND rail | INA219 #1 GND | Black |
| GND rail | INA219 #2 GND | Black |
| GND rail | BH1750 GND | Black |
| GND rail | OLED GND | Black |
| GND rail | DS18B20 GND | Black |
| ESP32 GPIO21 | Common SDA rail → all 4 I2C devices SDA | Yellow |
| ESP32 GPIO22 | Common SCL rail → all 4 I2C devices SCL | Blue |
| DS18B20 DATA | ESP32 GPIO4 | Green |
| 4.7 kΩ | DS18B20 DATA ↔ 3.3 V | (pull-up) |
| Test panel (+) | INA219 #1 **VIN+** | Red (22 AWG) |
| INA219 #1 **VIN−** | Load resistor (test) top | Red |
| Load resistor (test) bottom | Common GND | Black |
| Ref panel (+) | INA219 #2 **VIN+** | Red (22 AWG) |
| INA219 #2 **VIN−** | Load resistor (ref) top | Red |
| Load resistor (ref) bottom | Common GND | Black |
| ESP32 GPIO25 | Buzzer driver (base resistor) | Orange |
| ESP32 GPIO26 | 220 Ω → LED (+) | Orange |
| LED (−) | GND | Black |
| ESP32 GPIO27 | Relay IN (optional) | Orange |
| Relay COM/NO | Pump supply (+) (optional) | (separate) |
| Pump (−) | Pump supply (−) | (separate) |
| Pump supply (−) | ESP32 GND | Black (**common ground**) |

## 2.8 Connection Diagrams

### 2.8.1 Overall power + measurement wiring

```
   TEST PANEL (dusty)                    REFERENCE PANEL (clean)
   ┌──────────────┐                      ┌──────────────┐
   │ (+) ○────────┼── VIN+  INA219 #1  VIN− ──┐         │
   │ (−) ○────────┼── common GND        │      R_load   │
   └──────────────┘   (0x40)            │      (test)   │
                                        └──────┬───────┘
                                               │ GND
   ┌──────────────┐                            │
   │ (+) ○────────┼── VIN+  INA219 #2  VIN− ──┐│
   │ (−) ○────────┼── common GND              │ R_load   │
   └──────────────┘   (0x41)                  │  (ref)   │
                                              └────┬─────┘
                                                   │ GND
   ┌───────────────────────────────────────────────┴──────────────┐
   │                        COMMON GND RAIL                      │
   └──────────────┬───────────────────────────────────────────────┘
                  │
   ┌──────────────┴─────────────┐      ┌──────────────────────────┐
   │  ESP32 (5 V from USB)      │      │  I2C devices (3.3 V)     │
   │  SDA=21  SCL=22  1W=4      │◀────▶│  INA219×2, BH1750, OLED  │
   │  buzzer=25  led=26  rl=27  │      └──────────────────────────┘
   └────────────────────────────┘
```

### 2.8.2 One INA219 in detail (high-side series connection)

```
                    ┌──────────────── INA219 ────────────────┐
                    │                                        │
   PANEL (+) ───────┼──►[ VIN+ ]   [ VIN− ]──┬── R_load ─────┼──── PANEL (−) / GND
                    │      │                 │               │
                    │      └── internal 0.1 Ω shunt          │
                    │      VCC=3.3V  GND=GND  SDA=21  SCL=22│
                    └────────────────────────────────────────┘

   Current path: panel → shunt → load → ground
   Measured:  V across shunt ⇒ I = Vshunt / 0.1 Ω
              V on VIN−  ⇒ bus voltage
```

> **Do not** wire the INA219 in the panel's ground leg (low-side) here — high-side keeps the load referenced to ground and matches the wiring table above.

### 2.8.3 Local outputs

```
   GPIO25 ───[1 kΩ]───┤ B │      B = 2N2222 / BC547 (NPN)
                       ├ E ├── GND
                       │
   buzzer(+) ──────────┤ C │
   buzzer(−) ──────────┴──── 5 V      (magnetic buzzer: add flyback diode)

   GPIO26 ───[220 Ω]───►│├──── GND     LED (long leg = anode to resistor)
                         LED

   GPIO27 ───▶ relay IN        relay VCC = 5 V, GND = GND (opto-isolated module)
              relay COM/NO ──▶ pump(+)
              pump(−) ──▶ pump supply(−)  and pump supply(−) common with ESP32 GND
              [1N4007 across pump terminals, band to +]
```

## 2.9 Load Resistor Selection (formula + table)

```
                 Vmp          (panel voltage at max power)
   R_load  =  ────────   ≈   ─────────────────────────────
                 Imp          (panel current at max power)

   Power dissipated ≈ Pmax  →  choose a resistor rated ≥ 1.5× Pmax (2× is better)
```

| Common small panel | Vmp / Imp | R value | Minimum rating | Recommended buy |
| --- | --- | --- | --- | --- |
| 6 V / 2 W | 6 V / 0.33 A | **18 Ω** | 3 W | 18 Ω 10 W cement |
| 6 V / 3 W | 6 V / 0.5 A | **12 Ω** | 4.5 W | 12 Ω or 10 Ω 10 W cement |
| 12 V / 5 W | 12 V / 0.42 A | **28 Ω** | 7.5 W | 27 Ω 20 W |
| 18 V / 20 W | 18 V / 1.1 A | **16 Ω** | 30 W | ⚠ check **Voc < 26 V** for INA219 |

- **Read the label on your own panel** — don't guess.
- Use **exactly the same value on both panels**, and buy them from the same pack.
- Resistor temperature is normal; mount cement/wire-wound resistors off the plastic (they get hot).
- Alternative if you lack the right value: two resistors in parallel/series to hit it; keep both sides identical.

## 2.10 Power Budget

| Consumer | Typical | Peak (Wi-Fi TX) | Supply |
| --- | --- | --- | --- |
| ESP32 DevKit | 80-150 mA | **300-500 mA** | 5 V USB adapter |
| INA219 ×2 | ~1 mA | ~1 mA | 3.3 V (from ESP32) |
| BH1750 | ~0.2 mA | ~0.2 mA | 3.3 V |
| DS18B20 | ~1 mA | 1.5 mA (converting) | 3.3 V |
| OLED (typical drawing) | ~20 mA | ~25 mA | 3.3 V |
| Buzzer | 10-30 mA | 30 mA | 5 V / transistor |
| Relay coil (optional) | 70-90 mA | inrush | 5 V separate or module supply |
| **Total electronics** | **~120-250 mA** | **~500-700 mA** | **Use a 5 V / 1 A adapter (2 A if adding the relay)** |
| Panels → load resistors | ~0.5 A each | — | Independent; **not** used for electronics |

## 2.11 Mechanical Design

```
                         SUN (south side; India: azimuth = true south)
                          ☀  ↘  ↓  ↙
                        ┌──────────────────────┐
                        │   TEST PANEL (dusty) │      tilt θ ≈ local latitude
                        └──────────┬───────────┘      (e.g. 17° Bengaluru,
      side view:                    │                     28° Delhi)
   ════════════╗           ┌────────┴────────┐
               ║           │ REFERENCE PANEL │   ← same tilt, same azimuth
   ground/roof ╠═══════════╧═════════════════╧════
               ║      [ BH1750 flat, clear sky view ]
               ║
               ║      [ enclosure: ESP32, INA219s, OLED, buzzer ]
               ╚══════════════════════════════════════════

   TOP VIEW
   ┌────────────┐   gap ≥ 0.5-1 m (≥ 1 panel height)
   │ TEST PANEL │  ░░░░░░░░░  ┌────────────┐
   └────────────┘  ░ no shade ░│ REF  PANEL │
                 ░  allowed  ░ └────────────┘
                    ┌──────────┐
                    │ BH1750   │  ← flat, horizontal, unobstructed
                    │enclosure │  ← shaded/ventilated, wires strain-relieved
                    └──────────┘
```

| Design rule | Why |
| --- | --- |
| Same tilt & azimuth for both panels | The ratio is only valid if both see the identical sun angle |
| Tilt ≈ local latitude | Maximises annual yield (and gives a "professional" look in the report) |
| Gap ≥ one panel height; check at sunrise/sunset for mutual shading | Even a small shadow destroys the comparison |
| BH1750 flat with a clear sky view, not inside the box | Measures the sunlight the panels see |
| DS18B20 on the **back** of the test panel (thermal tape) | Measures panel temperature, not air temperature |
| Enclosure out of direct sun, ventilated, wires with strain relief | Protects electronics; heat shortens life |
| Reference panel discipline: dry-wipe weekly, log it | If the reference dirties, the index drifts toward 0 (a false "all clean") |

## 2.12 Enclosure & Weatherproofing

| Item | Practice |
| --- | --- |
| Box | Plastic ABS box with a gasket (aim IP54: rain-proof and dust-protected) |
| Cable entry | Cable glands (PG7/PG9) or rubber grommets — never bare holes |
| Moisture | Silica-gel sachet inside; drill 1-2 small "breather" holes downward-facing |
| Heat | OLED/ESP32 run fine up to ~50 °C ambient; keep the box shaded, not sealed in direct sun |
| Strain relief | Knot/tie the cables inside so a tug doesn't rip the breadboard |
| Final build | Move from breadboard to perfboard + screw terminals for vibration survival |
| Optional pump | Pump and tubing routed downward, drip loop in the cable, tubing fixed with clips |

## 2.13 Assembly Procedure (step by step)

**Stage A — bench (indoors, no panels yet)**
1. Fit the ESP32 to the breadboard; upload a **Blink** sketch; open Serial Monitor at 115200 — confirm the board and cable work.
2. Wire **only the I2C devices** (INA219 ×2, BH1750, OLED) to 3.3 V/GND/SDA/SCL; run the **I2C scanner** (Appendix B) — expect `0x23, 0x3C, 0x40, 0x41`.
3. Add the **DS18B20** with its 4.7 kΩ pull-up; read the temperature and compare with a thermometer (±1 °C).
4. Add **buzzer + LED**; drive them from a test sketch.
5. Add the **relay** (if used) and click it on/off; confirm the pump (or a lamp) switches.

**Stage B — electrical (with panels, still on the bench)**
6. Compute `R_load` from the panel label (§ 2.9) and solder/attach both.
7. Connect **panel → INA219 VIN+/VIN− → load → GND** for both panels; verify polarity with the multimeter (VIN+ toward the panel +).
8. In full sun (or a bright lamp), compare INA219 readings with the multimeter: voltage within ~2 %, current within ~5 %.
9. If a current reads negative → **swap VIN+/VIN−** on that INA219.

**Stage C — mechanics & enclosure**
10. Mount both panels on one frame: same tilt, same direction, adequate gap; mark the tilt with a protractor.
11. Stick the DS18B20 probe to the back of the **test** panel; place the BH1750 flat nearby.
12. Transfer the wiring to perfboard/screw terminals inside the enclosure; add glands and strain relief; add the silica gel.
13. Route power: 5 V adapter → ESP32; confirm the ESP32 does **not** draw from the panels.

**Stage D — field**
14. Power up outdoors; run the I2C scan again (heat/vibration can expose loose wires).
15. Perform **calibration with both panels spotless** (Part III § 3.7.6) and record `k`.
16. Leave it running for 24 h; check the dashboard is receiving data every 20 s.

## 2.14 Hardware Tests & Acceptance Criteria

| # | Test | Method | Pass criteria |
| --- | --- | --- | --- |
| H1 | Board alive | Blink + Serial print | LED blinks, serial text at 115200 |
| H2 | I2C discovery | Scanner sketch | All of 0x23, 0x3C, 0x40, 0x41 found |
| H3 | Voltage accuracy | INA219 vs multimeter | Within 2 % |
| H4 | Current accuracy | INA219 vs multimeter in series | Within 5 % |
| H5 | Polarity | Reverse load direction test | Positive reading, no negative values |
| H6 | Temperature | DS18B20 vs thermometer | Within 1 °C; never −127 |
| H7 | Lux sanity | Indoors vs full sun | Indoors ~300-1 000 lx; sun > 20 000 lx |
| H8 | Load resistor | Panel V/I ≈ label values | P within ~10 % of label Pmax |
| H9 | Local outputs | Force alert test | Buzzer + LED + OLED status switch together |
| H10 | Mechanical | Move/tilt the rig | No loose wires, no intermittent I2C |
| H11 | Power stability | Wi-Fi TX while running | No resets; add 100 µF cap across 3.3 V if flaky |
| H12 | Thermal | After 1 h in sun | Enclosure < ~50 °C, ESP32 stable |

**Hardware sign-off:** H1-H12 all pass → move to firmware (Part III).

## 2.15 Hardware Troubleshooting

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| I2C scan finds nothing | SDA/SCL swapped, no common GND, no 3.3 V | Recheck wiring; share ground; measure 3.3 V at the module |
| Only some devices found | Loose jumper / wrong address | Reseat wires; run scanner after each device |
| Both INA219s at 0x40 | A0 pad not bridged | Bridge A0→VCC on the reference-panel unit (0x41) |
| Current zero/negative | Shunt wired backwards | Swap VIN+ and VIN− |
| Current implausible | Load resistor missing/open | Panel needs a load; check the resistor value & joints |
| DS18B20 = −127 °C | Missing 4.7 kΩ pull-up | Fit pull-up DATA→3.3 V; check the probe wire |
| OLED blank | Wrong address (0x3D) or 5 V on VCC | Scan the bus; use 3.3 V |
| ESP32 resets randomly | Insufficient 5 V supply / relay inrush | Use ≥ 1 A adapter, separate relay/pump supply, add a 470 µF cap |
| Readings jump wildly | Noise, long unshielded wires, poor ground | Shorten wires, twist pairs, single ground point, average more samples |
| One panel always reads lower | Panel mismatch or partial shade | Verify identical models & tilt, check for shadows at test time |

---
---

# Part III — Software Part

> **Goal of this part:** firmware that reads every sensor, computes the soiling index correctly, decides when to alarm, shows it locally, and uploads it — reliably, without freezing when Wi-Fi or sunlight is missing.
> **Acceptance line:** *"Software is done when test mode gives the expected index and alerts, readings reach the dashboard every 20 s, and a Wi-Fi drop recovers by itself."*

## 3.1 Software Goals & Principles

| Principle | What it means here |
| --- | --- |
| **Modular** | Each job (read / compute / decide / display / upload) is its own function → testable and explainable |
| **Non-blocking** | No `delay()` in the main loop; everything scheduled with `millis()` → Wi-Fi never stalls sensing |
| **Fail-soft** | No Wi-Fi → keep measuring + OLED; no sun → skip evaluation; no sensor → show error, don't crash |
| **Data validity first** | Never compute a soiling number you cannot trust (light gate + current gate + staleness reset) |
| **Observable** | Serial log of every cycle so you can prove behaviour in your report |
| **Configurable** | All thresholds are named constants at the top of the file |

## 3.2 Toolchain Setup (do this once)

**Option A — Arduino IDE (recommended for beginners)**
1. Install **Arduino IDE 2.x**.
2. `File → Preferences → Additional Boards Manager URLs`:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. `Tools → Board → Boards Manager` → search **esp32** by Espressif → **Install**.
4. `Tools → Board` → choose **"ESP32 Dev Module"**; `Tools → Upload Speed` → 115200 (if uploads fail); `Tools → Port` → your COM port.
5. `Sketch → Include Library → Manage Libraries` → install **exactly these**:

| # | Library (search term) | Author | Used for |
| --- | --- | --- | --- |
| 1 | **Adafruit INA219** | Adafruit | Both V/I sensors (also pulls in Adafruit BusIO) |
| 2 | **BH1750** | Christopher Laws (mcltech) | Light sensor |
| 3 | **OneWire** | Paul Stoffregen | 1-Wire bus |
| 4 | **DallasTemperature** | Miles Burton | DS18B20 (needs OneWire) |
| 5 | **Adafruit SSD1306** | Adafruit | OLED (pulls in Adafruit GFX) |
| 6 | **ThingSpeak** | MathWorks | Cloud upload (skip if using Blynk/Firebase) |
| — | *WiFi, Wire, Preferences* | built-in | no install needed |

**Option B — PlatformIO (better for teams/version control)** — `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
  adafruit/Adafruit INA219@^1.5.1
  claws/BH1750@^1.3.0
  paulstoffregen/OneWire@^2.3.8
  milesburton/DallasTemperature@^3.11.0
  adafruit/Adafruit SSD1306@^2.5.9
  mathworks/ThingSpeak@^2.0.0
```

**Simulation first (optional):** Wokwi can simulate the ESP32 + INA219 + BH1750 + SSD1306 with a `diagram.json`. Use it to debug code before the sun cooperates.

## 3.3 Project Structure

```
solar-health-monitor/                 (Arduino IDE: single folder, main .ino inside)
├── solar-health-monitor.ino          ← the full firmware (§ 3.9)
├── README.md                         ← how to build & run (from this document)
├── docs/
│   ├── wiring.md                     ← § 2.7-2.8 tables & diagrams
│   ├── report/                       ← final report (structure in § 5.6)
│   └── data/                         ← test logs (CSV, § 4.6)
├── tools/
│   └── i2c_scanner.ino               ← Appendix B
└── dashboard/
    └── widgets.md                    ← § 3.13 layout notes
```

## 3.4 Software Architecture (module view)

```
┌──────────────────────────────────────────────────────────────────────────┐
│                            loop()  (forever)                             │
│                                                                          │
│   handleSerial() ──▶ calibrate / test-mode / print  (user commands)      │
│                                                                          │
│   every 5 s   :  readSensors() ─▶ computeSoiling() ─▶ updateAlert()      │
│                    │                                    │                │
│                    └──▶ updateOLED()  printStatus()      │                │
│                                                          ▼                │
│   every loop :  updateOutputs()  (buzzer/LED pattern from alarm flag)    │
│                                                                          │
│   every 20 s :  wifiEnsure() ─▶ upload() ─▶ ThingSpeak (7 fields)        │
└──────────────────────────────────────────────────────────────────────────┘

   State shared by the modules:
     Reading (vT,iT,vR,iR,pT,pR,lux,tC,soil,valid)   ← sensor/derived values
     k, calibrated                                    ← flash-stored calibration
     state (OK/ALERT/FAULT/OVERHEAT), alarmActive     ← decision outputs
     rolling window[15]                               ← 5-minute average
```

## 3.5 Program Flow & State Machine

### 3.5.1 Startup flow

```
            ┌──────────────┐
            │   POWER ON   │
            └──────┬───────┘
                   ▼
        ┌─────────────────────┐
        │ Serial(115200)      │
        │ pins as OUTPUT=LOW  │
        │ Wire.begin(21,22)   │
        └─────────┬───────────┘
                  ▼
        ┌─────────────────────┐   not found
        │ init INA219#1,#2,   │──────────────▶ print "ERR: sensor" on
        │ BH1750, DS18B20,    │                Serial + OLED, keep running
        │ OLED 0x3C           │                (degraded mode)
        └─────────┬───────────┘
                  ▼
        ┌─────────────────────┐
        │ load k + calOK flag │  Preferences ("calib")
        │ from flash          │
        └─────────┬───────────┘
                  ▼
        ┌─────────────────────┐   not calibrated
        │ Wi-Fi connect       │──────────────────▶ OLED shows "CALIBRATE ME"
        │ (15 s timeout)      │                    when sun is good (serial 'c')
        └─────────┬───────────┘
                  ▼
        ┌─────────────────────┐
        │ enter main loop     │  ← works ONLINE or OFFLINE from here
        └─────────────────────┘
```

### 3.5.2 Main loop (every cycle)

```
 ┌────────────────────────── 5 s tick ───────────────────────────┐
 │ read V,I ×2 (8 samples each, averaged) + lux + temperature   │
 │        ▼                                                      │
 │ P_test = V×I, P_ref = V×I                                     │
 │        ▼                                                      │
 │ valid?  (lux > 20 000  AND  I_ref > 5 mA)                     │
 │    ├─ NO ──▶ soil = n/a, show "LOW LIGHT", don't touch window │
 │    └─ YES ─▶ soil = (1 − (I_test/I_ref)/k) × 100              │
 │               push into 15-sample rolling window              │
 │        ▼                                                      │
 │ fault check → overheat check → soiling alert (avg >15 / <8)   │
 │        ▼                                                      │
 │ update OLED + buzzer + LED + serial log                        │
 └────────────────────────────────────────────────────────────────┘
 ┌────────────────────────── 20 s tick ──────────────────────────┐
 │ Wi-Fi retry if dropped ─▶ POST 7 fields to ThingSpeak        │
 └───────────────────────────────────────────────────────────────┘
 ┌────────────────────────── every loop ─────────────────────────┐
 │ handleSerial() · updateOutputs() (beep pattern, no delay())   │
 └───────────────────────────────────────────────────────────────┘
```

### 3.5.3 Alert state machine (the "brain" of the project)

```
                          ┌──────────────────────────────────────────┐
                          │            (boot / after reset)         │
                          └───────────────────┬──────────────────────┘
                                              ▼
              window < 15 samples      ┌────────────┐
     ┌────────────────────────────────▶│     OK     │◀──────────────┐
     │   (still OK, just unconfirmed)  └─────┬──────┘               │
     │                                       │ avg ≥ 15% for 5 min  │ avg < 8%
     │          ┌──────────────┐             ▼                      │
     │          │   FAULT      │                            ┌───────┴──────┐
     └──────────│ I_ref normal,│                            │    ALERT     │
                │ I_test ≈ 0   │                            │ buzzer+LED+  │
                └──────────────┘                            │ push + OLED  │
                ┌──────────────┐                            └──────────────┘
                │  OVERHEAT    │   (any state with T > 65 °C takes priority
                └──────────────┘    over ALERT; it returns to OK/ALERT when T recovers)

  Evaluation is SKIPPED (state held) while lux < 20 000 → no false alarms at
  night/cloud. Hysteresis 15% ON / 8% OFF prevents flicker.
```

**State table (what each state does)**

| State | Code (field 7) | Enters when | Leaves when | Outputs |
| --- | --- | --- | --- | --- |
| OK | 0 | Default; or avg < 8 % | avg ≥ 15 % for a full 5-min window | OLED "OK", LED/buzzer off |
| ALERT | 1 | avg ≥ 15 % with window full | avg < 8 % | OLED "CLEANING!", pulsed buzzer, LED, push notification |
| FAULT | 2 | I_ref > 50 mA **and** I_test < 5 mA | test-panel current returns | OLED "FAULT!", different message on dashboard |
| OVERHEAT | 3 | T_panel > 65 °C | T back below threshold | OLED "OVERHEAT!", warning notification |
| *(low light)* | — | lux < 20 000 | lux ≥ 20 000 | Evaluation skipped; OLED shows "LOW LIGHT" |

## 3.6 Module / Function Inventory

| Function | Responsibility | Key inputs | Key outputs | When |
| --- | --- | --- | --- | --- |
| `setup()` | Init buses, sensors, OLED, flash, Wi-Fi | — | ready state | once |
| `readAveraged()` | Average N samples of one INA219 | sensor object | v (V), i (mA) | each read |
| `readSensors()` | All readings (or fake in test mode) | testMode | `Reading` struct | every 5 s |
| `computeSoiling()` | Power, validity gate, soiling %, window push | `Reading`, k | `r.soil`, `r.valid` | every 5 s |
| `windowAvg()` | Mean of stored window | window | avg % | every 5 s |
| `updateAlert()` | FSM: fault / overheat / soiling + hysteresis | `Reading`, avg | `state`, `alarmActive` | every 5 s |
| `updateOutputs()` | LED + pulsed buzzer from alarm flag | alarmActive | GPIO | every loop |
| `updateOLED()` | Two-screen local UI | `Reading`, state | display | every 5 s-1 s |
| `wifiEnsure()` | Reconnect Wi-Fi every 60 s if dropped | WiFi status | connected? | every 20 s |
| `upload()` | POST 7 fields to ThingSpeak | `Reading`, state | HTTP result | every 20 s |
| `calibrate()` | Compute & store `k` (both panels clean) | `Reading` | saved to flash | serial 'c' |
| `handleSerial()` | CLI: c/t/p/x/r commands | serial input | actions | every loop |
| `printStatus()` | One-line CSV-ish log for the report | `Reading` | Serial | every 5 s |

## 3.7 Algorithms in Detail

### 3.7.1 Sample averaging (noise rejection)
Take `N_SAMPLES = 8` readings of each INA219, 15 ms apart (~120 ms per sensor pair), and average. INA219 LSB noise of a few mA disappears; a single glitch can't fake an alert.

### 3.7.2 Validity gate (the false-alarm killer)
```
valid = (lux > 20 000 lx)  AND  (I_ref > 5 mA)
```
- **Lux gate:** at night/dawn/cloud, `I_ref` is tiny and the ratio becomes meaningless noise.
- **Current gate:** if the reference produces ~0 mA, dividing by it explodes.
- Invalid ⇒ *do not compute, do not push to the window, do not change the alert state.*

### 3.7.3 Soiling index
```
soil(%) = (1 − (I_test / I_ref) / k) × 100        clamped to [−10, +100]
```
Clamping keeps a nonsense value (e.g. −400 % from a sensor glitch) from poisoning the average.

### 3.7.4 Rolling window (5-minute confirmation)
- Circular buffer of the last `WINDOW = 15` **valid** samples (15 × 20 s ≈ 5 min of valid data).
- Alert requires the window to be **full** *and* the mean ≥ 15 % → a momentary shadow or a bird can never trigger it.
- **Staleness reset:** if no valid sample arrives for 10 minutes (sunset, long cloud), the window is emptied, so yesterday's dusty reading cannot hold tonight's alert.

### 3.7.5 Hysteresis
```
ON  when mean > 15 %   (with full window)
OFF when mean <  8 %
in between → keep previous state
```
This is the classic "Schmitt trigger" behaviour: one clean transition, no flicker.

### 3.7.6 Calibration (one-time, stored in flash)
**Preconditions:** both panels spotless, strong sun (lux > 20 000), readings stable.
```
k = I_test / I_ref        → saved in Preferences namespace "calib", key "k", flag "calOK"
```
- Survives reboot/power loss (Preferences = ESP32 NVS flash).
- Re-run whenever you replace a panel or suspect mismatch.
- Sanity check: `k` should be **0.95 - 1.05**. `k = 0.70` means you calibrated with a dirty test panel or a shaded one.

### 3.7.7 Fault detection
```
if (I_ref > 50 mA  AND  I_test < 5 mA) → FAULT
```
Meaning: the sun is definitely strong (reference is producing), but the test panel is dead → disconnected wire, failed panel, or it is fully shaded. This distinguishes *"panel broken"* from *"weather bad"*.

### 3.7.8 Non-blocking scheduling
Every periodic job stores its last-run time and compares with `millis()`:
```
if (millis() - tUpload >= UPLOAD_MS) { tUpload = millis(); upload(); }
```
No `delay()` in the loop ⇒ the buzzer keeps blinking and sensors keep reading even while Wi-Fi is connecting/retrying.

### 3.7.9 Wi-Fi resilience
1. Boot: try 15 s, then continue **offline** (never brick the node because the router is down).
2. Every 20 s tick: if not connected and 60 s since last try → `WiFi.disconnect(); WiFi.begin(...)`.
3. Uploads are **skipped** (not queued) while offline — the last values are still visible locally; ThingSpeak history resumes on reconnect.

## 3.8 Configuration Constants (all tunables in one place)

| Constant | Default | Meaning | How to tune |
| --- | --- | --- | --- |
| `LUX_MIN` | 20 000 lx | Below this, ignore soiling | Raise for hazy sites; keep well above dawn/dusk levels |
| `SOIL_ALERT_ON` | 15 % | Alert trigger | Lower to 10 % if cleaning is very cheap; raise to 20 % if lazy |
| `SOIL_CLEAR_OFF` | 8 % | Alert clear | Keep ~half of ON |
| `TEMP_WARN_C` | 65 °C | Overheat warning | 60-70 °C typical |
| `IREF_MIN_MA` | 5 mA | Minimum reference current for a valid ratio | Raise to 10-20 mA for noisy setups |
| `IREF_FAULT_MA / ITEST_FAULT_MA` | 50 / 5 mA | Fault detection window | Proportional to your panel's Imp |
| `N_SAMPLES` | 8 | Samples averaged | 4 = faster, 16 = smoother |
| `WINDOW` | 15 samples | = 5 min confirmation | Change with `UPLOAD_MS`/`READ_MS` |
| `READ_MS` | 5 000 ms | Sensor read period | 2-10 s |
| `UPLOAD_MS` | 20 000 ms | Cloud upload period | **Never below 15 s on ThingSpeak free tier** |
| `WIFI_RETRY_MS` | 60 000 ms | Reconnect back-off | — |
| `WIFI_SSID / WIFI_PASS` | yours | Credentials | 2.4 GHz network! |
| `TS_CHANNEL / TS_WRITE_KEY` | yours | ThingSpeak IDs | From the channel page |

## 3.9 Complete Firmware (ESP32 / Arduino)

> Single-file reference firmware — paste into `solar-health-monitor.ino`. It implements everything above: averaging, validity gate, soiling math, rolling window, staleness reset, fault/overheat states, hysteresis, OLED UI, buzzer/LED, non-blocking Wi-Fi + ThingSpeak upload, flash-persisted calibration, a serial CLI, and a deterministic test mode.

```cpp
/* =========================================================================
 *  SMART SOLAR PANEL HEALTH & DUST MONITORING SYSTEM — firmware v1.0
 *  Target : ESP32 DevKit (Arduino core for ESP32)
 *  Cycle  : read/decide every 5 s, cloud upload every 20 s, no delay()
 * -------------------------------------------------------------------------
 *  Serial CLI:  'c' calibrate (panels must be clean & in sun)
 *               't' toggle test mode (fake data: soil = 20 %)
 *               'p' print one status line now
 *               'x' erase stored calibration
 *               'r' reset the 5-minute rolling window
 * ========================================================================= */

#include <Wire.h>
#include <WiFi.h>
#include <Preferences.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ThingSpeak.h>

/* ---------------------------- 1. PIN MAP -------------------------------- */
#define PIN_SDA       21
#define PIN_SCL       22
#define PIN_ONEWIRE    4
#define PIN_BUZZER    25
#define PIN_LED       26
#define PIN_RELAY     27        // optional cleaning pump

/* --------------------------- 2. TUNABLES -------------------------------- */
static const float    LUX_MIN        = 20000.0f; // lx : below this, don't evaluate
static const float    SOIL_ALERT_ON  =    15.0f; // %  : alert ON  threshold
static const float    SOIL_ALERT_OFF =     8.0f; // %  : alert OFF threshold
static const float    TEMP_WARN_C    =    65.0f; // °C : overheat warning
static const float    IREF_MIN_MA    =     5.0f; // mA : min reference current to trust
static const float    IREF_FAULT_MA  =    50.0f; // mA : reference healthy ...
static const float    ITEST_FAULT_MA =     5.0f; // mA : ... but test panel dead => FAULT
static const int      N_SAMPLES      =     8;    // samples per averaged reading
static const int      WINDOW         =    15;    // 15 valid samples ≈ 5 min
static const uint32_t READ_MS        =  5000UL;  // sensor read period
static const uint32_t UPLOAD_MS      = 20000UL;  // ThingSpeak free tier ≥ 15 s
static const uint32_t WIFI_RETRY_MS  = 60000UL;
static const uint32_t STALE_MS       = 600000UL; // clear window after 10 min invalid

/* ----------------------- 3. CREDENTIALS / IDS --------------------------- */
static const char*         WIFI_SSID   = "YOUR_WIFI";
static const char*         WIFI_PASS   = "YOUR_PASSWORD";
static const unsigned long TS_CHANNEL  = 0;             // ThingSpeak channel ID
static const char*         TS_WRITE_KEY= "YOUR_WRITE_KEY";

/* ---------------------------- 4. OBJECTS -------------------------------- */
Adafruit_INA219   inaTest(0x40);   // test  panel
Adafruit_INA219   inaRef (0x41);   // reference panel (A0 bridged to VCC)
BH1750            lightMeter;
OneWire           oneWire(PIN_ONEWIRE);
DallasTemperature tempSensor(&oneWire);
Adafruit_SSD1306  oled(128, 64, &Wire, -1);
WiFiClient        wifiClient;
Preferences       prefs;

/* ---------------------------- 5. STATE ---------------------------------- */
enum NodeState { ST_OK = 0, ST_ALERT = 1, ST_FAULT = 2, ST_OVERHEAT = 3 };

struct Reading {
  float vT = 0, iT = 0, vR = 0, iR = 0;   // volts, milliamps
  float pT = 0, pR = 0;                    // watts
  float lux = 0, tC = 0;                   // lx, °C
  float soil = 0;                          // %
  bool  valid = false;
};

Reading    last;
float      k = 1.0f;
bool       calibrated = false;
bool       testMode   = false;
bool       soilAlert  = false;             // soiling-specific alert flag
NodeState  state      = ST_OK;
bool       alarmActive= false;
float      win[WINDOW];
int        winIdx = 0, winFilled = 0;
uint32_t   tRead = 0, tUpload = 0, tWifi = 0, lastValidMs = 0;

/* ======================= 6. SENSOR ACQUISITION =========================== */
void readAveraged(Adafruit_INA219 &ina, float &v, float &i) {
  float vs = 0, is = 0;
  for (int n = 0; n < N_SAMPLES; n++) {
    vs += ina.getBusVoltage_V();
    is += ina.getCurrent_mA();
    delay(15);                       // only 15 ms × 8 — read() is quick overall
  }
  v = vs / N_SAMPLES;
  i = fabs(is / N_SAMPLES);          // tolerate reversed shunt wiring
}

void readSensors(Reading &r) {
  if (testMode) {                    // deterministic demo values (no sun needed)
    r.vT = 5.80f;  r.vR = 5.95f;
    r.iT = 400.0f; r.iR = 500.0f;    // ratio 0.80, k = 1.00  → soiling = 20 %
    r.lux = 40000.0f;
    r.tC  = 41.0f;
  } else {
    readAveraged(inaTest, r.vT, r.iT);
    readAveraged(inaRef,  r.vR, r.iR);
    r.lux = lightMeter.readLightLevel();
    tempSensor.requestTemperatures();
    r.tC  = tempSensor.getTempCByIndex(0);   // −127 ⇒ wiring error (ignored below)
    if (r.tC < -50.0f) r.tC = 0.0f;
  }
  r.pT = r.vT * r.iT / 1000.0f;
  r.pR = r.vR * r.iR / 1000.0f;
}

/* =================== 7. SOILING MATH + ROLLING WINDOW ==================== */
void computeSoiling(Reading &r) {
  r.valid = (r.lux > LUX_MIN) && (r.iR > IREF_MIN_MA);
  r.soil  = 0.0f;
  if (r.valid) {
    lastValidMs = millis();
    r.soil = (1.0f - (r.iT / r.iR) / k) * 100.0f;
    if (r.soil < -10.0f) r.soil = -10.0f;    // clamp noise
    if (r.soil > 100.0f) r.soil = 100.0f;
    win[winIdx] = r.soil;
    winIdx = (winIdx + 1) % WINDOW;
    if (winFilled < WINDOW) winFilled++;
  } else if (millis() - lastValidMs > STALE_MS && winFilled > 0) {
    winIdx = 0; winFilled = 0;               // sunset: forget stale evidence
  }
}

float windowAvg() {
  if (winFilled == 0) return 0.0f;
  float s = 0;
  for (int i = 0; i < winFilled; i++) s += win[i];
  return s / winFilled;
}

/* ========================== 8. ALERT ENGINE ============================== */
void updateAlert(const Reading &r) {
  float avg  = windowAvg();
  bool  full = (winFilled >= WINDOW);         // 15 valid samples ≈ 5 minutes

  if (r.valid) {                              // hysteresis on the running mean
    if (full && avg > SOIL_ALERT_ON) soilAlert = true;
    if (        avg < SOIL_ALERT_OFF) soilAlert = false;
  }

  if      (r.iR > IREF_FAULT_MA && r.iT < ITEST_FAULT_MA) state = ST_FAULT;
  else if (r.tC > TEMP_WARN_C)                            state = ST_OVERHEAT;
  else                                                    state = soilAlert ? ST_ALERT : ST_OK;

  alarmActive = (state != ST_OK);
}

void updateOutputs() {                        // called every loop → smooth beep
  digitalWrite(PIN_LED, alarmActive);
  digitalWrite(PIN_BUZZER, alarmActive && ((millis() / 500) % 2));
}

/* ============================ 9. DISPLAY ================================= */
const char* stateText() {
  switch (state) {
    case ST_ALERT:    return "CLEANING!";
    case ST_FAULT:    return "FAULT!";
    case ST_OVERHEAT: return "OVERHEAT!";
    default:          return (last.lux < LUX_MIN) ? "LOW LIGHT" : "OK";
  }
}

void updateOLED(const Reading &r) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);

  oled.setCursor(0, 0);  oled.print("SOLAR HEALTH MONITOR");
  oled.drawFastHLine(0, 9, 127, SSD1306_WHITE);

  oled.setCursor(0, 12); oled.printf("T %5.2fV %5.0fmA %4.1fW", r.vT, r.iT, r.pT);
  oled.setCursor(0, 22); oled.printf("R %5.2fV %5.0fmA %4.1fW", r.vR, r.iR, r.pR);
  oled.setCursor(0, 32); oled.printf("Lux %6.0f  T %4.1fC", r.lux, r.tC);
  oled.setCursor(0, 42); oled.printf("Soil %5.1f%%  %s", windowAvg(), stateText());
  oled.setCursor(0, 52);
  oled.printf("k=%.3f %s %s", k,
              (WiFi.status() == WL_CONNECTED) ? "WIFI" : "----",
              testMode ? "TEST" : (calibrated ? "CAL" : "NEED-CAL"));

  if (!calibrated && !testMode) {             // prominent first-boot prompt
    oled.fillRect(0, 52, 128, 12, SSD1306_BLACK);
    oled.setCursor(0, 52);
    oled.print("CALIBRATE: serial 'c'");
  }
  oled.display();
}

/* ======================== 10. COMMUNICATION ============================== */
void wifiEnsure() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - tWifi < WIFI_RETRY_MS) return;
  tWifi = millis();
  Serial.println("[wifi] reconnecting...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void upload(const Reading &r) {
  if (WiFi.status() != WL_CONNECTED) { Serial.println("[cloud] skipped (no wifi)"); return; }
  ThingSpeak.setField(1, r.vT);          // volts, test
  ThingSpeak.setField(2, r.iT);          // mA, test
  ThingSpeak.setField(3, r.pT);          // W, test
  ThingSpeak.setField(4, r.lux);         // lx
  ThingSpeak.setField(5, r.tC);          // °C
  ThingSpeak.setField(6, windowAvg());   // soiling % (5-min mean)
  ThingSpeak.setField(7, (float)state);  // 0 OK, 1 ALERT, 2 FAULT, 3 OVERHEAT
  int rc = ThingSpeak.writeFields(TS_CHANNEL, TS_WRITE_KEY);
  if (rc == 200) Serial.println("[cloud] ok");
  else           Serial.printf("[cloud] error %d\n", rc);
}

/* ========================= 11. CALIBRATION =============================== */
bool calibrate(const Reading &r) {
  if (r.lux < LUX_MIN)               { Serial.println("[cal] FAIL: light too low");  return false; }
  if (r.iT < IREF_MIN_MA || r.iR < IREF_MIN_MA) {
                                       Serial.println("[cal] FAIL: current too low"); return false; }
  k = r.iT / r.iR;
  calibrated = true;
  prefs.putFloat("k", k);
  prefs.putBool ("calOK", true);
  Serial.printf("[cal] OK  k = %.4f  (both panels MUST have been clean)\n", k);

  oled.clearDisplay(); oled.setTextSize(2); oled.setCursor(10, 16);
  oled.printf("k=\n%.3f", k); oled.display();
  delay(2500);
  oled.setTextSize(1);
  return true;
}

/* ========================== 12. SERIAL CLI =============================== */
void printStatus(const Reading &r) {
  Serial.printf("[%lu] VT=%.2f IT=%.0f VR=%.2f IR=%.0f P=%.2f lux=%.0f T=%.1f "
                "soil=%.1f avg=%.1f valid=%d state=%d wifi=%d\n",
                millis() / 1000, r.vT, r.iT, r.vR, r.iR, r.pT, r.lux, r.tC,
                r.soil, windowAvg(), r.valid, state,
                WiFi.status() == WL_CONNECTED);
}

void handleSerial(Reading &r) {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'c': case 'C': calibrate(r);        break;
    case 't': case 'T':
      testMode = !testMode;
      Serial.printf("[test] mode %s\n", testMode ? "ON (soil=20%%)" : "OFF");
      break;
    case 'p': case 'P': printStatus(r);      break;
    case 'x': case 'X':
      prefs.remove("k"); prefs.remove("calOK");
      k = 1.0f; calibrated = false;
      Serial.println("[cal] erased");        break;
    case 'r': case 'R':
      winIdx = 0; winFilled = 0;
      Serial.println("[window] reset");      break;
  }
}

/* ============================= 13. SETUP ================================= */
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n== Smart Solar Health Monitor v1.0 =="));

  pinMode(PIN_BUZZER, OUTPUT); pinMode(PIN_LED, OUTPUT); pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW); digitalWrite(PIN_LED, LOW); digitalWrite(PIN_RELAY, LOW);

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!inaTest.begin()) Serial.println("[hw] ERR: INA219 #1 (0x40) missing");
  if (!inaRef.begin())  Serial.println("[hw] ERR: INA219 #2 (0x41) missing");
  // default INA219 calibration (32 V / 800 mA) suits panels with Imp < 0.8 A

  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  tempSensor.begin();
  tempSensor.setResolution(12);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("[hw] ERR: OLED (0x3C) missing");
  oled.clearDisplay(); oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0); oled.println("Booting...\nWiFi: connecting..."); oled.display();

  prefs.begin("calib", false);
  k = prefs.getFloat("k", 1.0f);
  calibrated = prefs.getBool("calOK", false);
  Serial.printf("[cal] stored k=%.3f  calibrated=%d\n", k, calibrated);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(250);
  Serial.println(WiFi.status() == WL_CONNECTED ? "[wifi] connected"
                                               : "[wifi] FAILED — running offline");
  ThingSpeak.begin(wifiClient);

  lastValidMs = millis();
  tRead = tUpload = millis();
}

/* ============================== 14. LOOP ================================= */
void loop() {
  handleSerial(last);

  if (millis() - tRead >= READ_MS) {          // ---- sense + decide ----
    tRead = millis();
    readSensors(last);
    computeSoiling(last);
    updateAlert(last);
    updateOLED(last);
    printStatus(last);
  }

  updateOutputs();                            // ---- local alarm pattern ----

  if (millis() - tUpload >= UPLOAD_MS) {       // ---- cloud ----
    tUpload = millis();
    wifiEnsure();
    upload(last);
  }
}
```

**What this code does NOT include (add for your final version, listed as "to-do" in the brief):**
- Relay/pump automation logic (drive `PIN_RELAY` when `state == ST_ALERT` + a pump timeout).
- E-mail/webhook alerts (use ThingSpeak **React/ThingHTTP** or Blynk notifications — § 3.12).
- Over-the-air (OTA) updates and MQTT — future scope.

## 3.10 OLED User Interface

**Screen 1 — normal (auto-shown every read):**

```
┌─────────────────────────── 128 × 64 ───────────────────────────┐
│ SOLAR HEALTH MONITOR                                           │
│ ─────────────────────────────────────────────────────────────  │
│ T  6.01V   486mA  2.9W                                         │
│ R  6.03V   494mA  3.0W                                         │
│ Lux  42150  T  41.6C                                           │
│ Soil   1.6%  OK                                                │
│ k=1.002 WIFI CAL                                               │
└────────────────────────────────────────────────────────────────┘
   T = test panel, R = reference panel, Soil = 5-minute mean
   status words: OK · LOW LIGHT · CLEANING! · FAULT! · OVERHEAT!
```

**Screen 2 — alarm / first boot:**

```
┌────────────────────────────────────────────────────────────────┐
│ SOLAR HEALTH MONITOR                                           │
│ ─────────────────────────────────────────────────────────────  │
│ T  5.80V   400mA  2.3W                                         │
│ R  5.95V   500mA  3.0W                                         │
│ Lux  40000  T  41.0C                                           │
│ Soil  20.0%  CLEANING!      ← inverted/flashing when alarming │
│ k=1.000 WIFI TEST                                              │
└────────────────────────────────────────────────────────────────┘
```

## 3.11 Serial Console & Log Format

**Boot output**
```
== Smart Solar Health Monitor v1.0 ==
[cal] stored k=1.002  calibrated=1
[wifi] connected
```

**Per-cycle line (5 s) — copy straight into your report's data table**
```
[120] VT=6.01 IT=486 VR=6.03 IR=494 P=2.92 lux=42150 T=41.6 soil=1.6 avg=1.4 valid=1 state=0 wifi=1
[125] VT=5.98 IT=401 VR=6.03 IR=495 P=2.40 lux=41880 T=42.0 soil=19.2 avg=9.8 valid=1 state=0 wifi=1
[130] VT=5.97 IT=399 VR=6.02 IR=493 P=2.38 lux=42010 T=42.3 soil=19.1 avg=15.1 valid=1 state=1 wifi=1
```
Columns: `time(s) | V test | mA test | V ref | mA ref | W test | lux | °C | soil% | 5-min avg% | valid | state | wifi`

## 3.12 Cloud Setup (ThingSpeak, step by step)

1. Create a free account at ThingSpeak (MathWorks) → **New Channel**.
2. Name it `Solar Health Monitor`; enable **8 fields** (7 used + 1 spare) and set names:
3. Copy **Channel ID** → `TS_CHANNEL` and **Write API Key** → `TS_WRITE_KEY` in the firmware.
4. Upload the firmware; open **Channel View** — data appears every 20 s.
5. Add widgets: **Gauge** (field 1 V), **Gauge** (field 2 mA), **Gauge** (field 3 W), **Gauge** (field 6 soiling %), **Numeric Display** (field 5 °C), **Line charts** (fields 1+2; field 6; field 4).
6. **Status tile:** use a *MATLAB visualisation* or simply chart field 7 and read the state code (0/1/2/3).
7. **Phone/e-mail alert:** use **React** (rule: "if field 6 > 15 → send e-mail") or **ThingHTTP** → a webhook to IFTTT/Blynk/push service. Platform UIs change often — follow the current documentation when you do this step.
8. **History/energy:** add a MATLAB analysis to integrate power × time into daily Wh.

**Field map (keep exactly this order)**

| Field | Value | Unit | Chart type |
| --- | --- | --- | --- |
| 1 | Test-panel voltage | V | gauge + line |
| 2 | Test-panel current | mA | gauge + line |
| 3 | Test-panel power | W | gauge + line (vs field via ref channel) |
| 4 | Light | lx | line |
| 5 | Panel temperature | °C | numeric display |
| 6 | Soiling index (5-min mean) | % | gauge + line (threshold line at 15) |
| 7 | Alert state | 0-3 | status/numeric |
| 8 | (spare) reference power / RSSI | W / dBm | line |

**Alternative platforms**

| Platform | Best for | How data gets there | Alerts |
| --- | --- | --- | --- |
| **ThingSpeak** (default) | Fast charts, MATLAB analysis | HTTP `writeFields` every 20 s | React / ThingHTTP → e-mail |
| **Blynk** | Real mobile app + push notifications | Blynk library, virtual pins V0-V6 | `Blynk.logEvent()` → push |
| **Firebase** | Fully custom web page | Firebase REST client → Realtime DB | Cloud Function → FCM push |

## 3.13 Dashboard Design

```
┌──────────────────────── Solar Panel Health ────────────────────────────┐
│ STATUS: [ CLEANING REQUIRED ]   Last update 12:00:20   Wi-Fi −54 dBm   │
├───────────────┬───────────────┬───────────────┬────────────────────────┤
│  6.01 V       │  486 mA       │  2.92 W       │  15.4 %                │
│   (gauge)     │   (gauge)     │   (gauge)     │  SOILING (gauge, red)  │
├───────────────┴───────────────┴───────────────┴────────────────────────┤
│ POWER vs TIME — test (solid) vs reference (dashed)                    │
│ 3.0 ┤      ~~~ref~~~~  ~~~ref~~~~   ~~~ref~~~                         │
│ 2.5 ┤ ____/¯¯¯¯¯¯\_____/¯¯¯¯¯¯\_____/¯¯¯¯¯¯\___  ← gap = dust loss     │
│ 2.0 ┤                                                                 │
│     └───────────────────────────────────────────── 00:00 … 24:00       │
├───────────────────────────────────────────────────────────────────────┤
│ SOILING % vs TIME — dashed line = 15 % alert threshold                │
│ 20 ┤                                  ╱‾‾‾‾‾                          │
│ 15 ┤ - - - - - - - - - - - - - - - - ╱- - - - - - - - (ALERT)         │
│  0 ┤______________╱─────────────────                                  │
├──────────────────────┬────────────────────────────────────────────────┤
│ Lux  42 150 lx       │ HISTORY: daily Wh · cleaning events (markers)  │
│ Temp 41.6 °C         │ "Cleaned 12 Mar — recovered 19 % output"       │
└──────────────────────┴────────────────────────────────────────────────┘
```

| Widget | Data | Purpose |
| --- | --- | --- |
| Status banner | field 7 | Instant "is everything OK?" |
| 4 gauges | V, mA, W, soiling % | Live numbers |
| Line chart 1 | test vs reference power | Visual proof of the dust gap |
| Line chart 2 | soiling % + 15 % threshold | Trend + when the alert fired |
| Numeric/gauge | lux, temperature | Context (why a reading is/ isn't valid) |
| Notification | push/e-mail on state 1 | The actual "go clean it" message |
| History | daily Wh, cleaning markers | "Gain after cleaning" evidence |

## 3.14 Test Mode & Software Test Cases

**Test mode:** press `t` in the Serial Monitor → sensors are replaced with fixed values (`I_test/I_ref = 0.80`, `lux = 40 000`) so the whole pipeline (20 % soiling → 5-min window → alert → upload) can be proven **without sunlight**. Press `t` again to return to live data.

| # | Test | How | Expected |
| --- | --- | --- | --- |
| S1 | Soiling math | Test mode ON, wait 5 min | `soil = 20.0 %`, state → 1 after window fills, buzzer/LED/OLED alert |
| S2 | Alert clears | Test mode ON, then `r` + switch back to clean live ratio | avg < 8 % → state 0, alarm stops |
| S3 | Low-light gate | Test mode: set `LUX_MIN` high or cover the BH1750 | `valid=0`, no window push, no state change, OLED "LOW LIGHT" |
| S4 | Fault state | Test mode values with `iR = 400, iT = 0` (edit temporarily) | state = 2, OLED "FAULT!" |
| S5 | Overheat | Test mode with `tC = 70` | state = 3, OLED "OVERHEAT!" |
| S6 | Calibration | Serial `x`, then `c` in strong sun with clean panels | prints `k ≈ 0.95-1.05`, stored (reboot keeps it) |
| S7 | Wi-Fi recovery | Change the password, upload, then fix it | Uploads skipped, then resume within 60 s |
| S8 | Reboot persistence | Reboot after calibration | `k` still stored, no re-calibration prompt |
| S9 | Long run | Leave running 24 h | One log line per 5 s, one upload per 20 s, no resets (check `rst:` in log) |
| S10 | No-crash checks | Unplug each sensor in turn | Error printed, node keeps running, reconnects when re-plugged (hot-plug I2C requires a reset) |

## 3.15 Software Acceptance Criteria

- [ ] Test mode yields exactly 20 % soiling and raises the alert after 5 minutes.
- [ ] Low-light input never changes the state.
- [ ] Alert clears only when the mean drops below 8 % (no flicker between 8 and 15 %).
- [ ] `k` survives power cycle; calibration refuses to run in poor light.
- [ ] Dashboard receives all 7 fields every 20 s (±2 s).
- [ ] Wi-Fi drop → automatic recovery without a reboot.
- [ ] 24-hour run with zero watchdog resets.
- [ ] Serial log shows `state` transitions with timestamps (evidence for the report).

## 3.16 Software Troubleshooting

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| `BH1750` returns 0 / 65535 | Wrong library or ADDR pin high | Use the claws/BH1750 library; check ADDR = GND |
| `lux` stuck at 0 | Sensor in direct dark / not initialised | `begin(CONTINUOUS_HIGH_RES_MODE)`; wait ~200 ms |
| Soiling always 0 | `k` = 1 and panels match, or `valid` always false | Print `valid`, `lux`, `iR` — check the light gate |
| Soiling always ~100 % | `iR` ≈ 0 (bad reference wiring) or wrong address | Verify 0x41 and the reference load resistor |
| Soiling noisy ±5 % | Few samples, shading, thin wires | Raise `N_SAMPLES`, check shading, shorten wires |
| Alert never fires | Window never fills (too many invalid samples) | Log `winFilled`; lower `LUX_MIN` only if justified |
| Alert never clears | Reference panel got dirty, or window holds stale data | Wipe the reference panel; check `STALE_MS` reset |
| `cloud error 400` | Wrong field/key or update < 15 s | Check key, keep `UPLOAD_MS ≥ 15000` |
| `cloud error 421` | Upload rejected (rate limit) | Increase interval |
| Uploads work once then stop | Blocking Wi-Fi code added in the loop | Keep `millis()` scheduling, no long `delay()` |
| Reboot during upload | Power sag (weak 5 V supply) | ≥ 1 A adapter, 470 µF cap across 5 V |
| OLED shows garbage | Wrong address or shared-bus glitch | Scan bus (0x3C), shorten I2C wires |
| Compile error on `printf` float | Wrong board selected | Tools → Board → "ESP32 Dev Module" |

---
---

# Part IV — Figures, Charts & Sample Data

> All figures below are **expected shapes** of your real data. Reproducing them with your own measurements is the core evidence that the project works.

## 4.1 Figure — Clean vs Dusty Output (same sun, same moment)

```
   Power at the load (W) — measured on the same sunny minute

   CLEAN reference  │ ██████████████████████████  3.00 W
   DUSTY test       │ ███████████████████████      2.40 W     (-20 %)

                    └──┬────┬────┬────┬────┬────┬────┬──
                       0   0.5  1.0  1.5  2.0  2.5  3.0  W
```

## 4.2 Figure — Soiling Index Builds Up Over a Week (cleaned on Day 7)

```
   Soiling index (%)
   25 ┤
   20 ┤                                       ▓▓▓▓▓▓▓▓  20 %  ← ALERT (avg>15 for 5 min)
   15 ┤ - - - - - - - - - - - - - - - - - - - - - - - - - - ALERT threshold
   13 ┤                                 ▓▓▓▓▓▓▓▓
   10 ┤                           ▓▓▓▓▓▓▓▓
    8 ┤ - - - - - - - - - - - - - - - - - - - - - - - - - - CLEAR threshold
    5 ┤                 ▓▓▓▓▓▓▓▓
    2 ┤        ▓▓▓▓▓▓▓▓
    0 ┤ ▓▓▓▓▓▓                                          ▓▓▓▓ (after cleaning)
      └──────────────────────────────────────────────────────
        Day1    Day2    Day3    Day4    Day5    Day6    Day7
        clean                                                   wiped
```

## 4.3 Figure — Sunlight Over One Day (why the light gate matters)

```
   Lux
   90k ┤                    ╱‾‾‾‾‾‾‾‾‾‾‾‾╲          ← BH1750 saturates ~65k:
   60k ┤                 ╱‾‾‾               ‾‾╲         still > 20 000, gate OPEN
   40k ┤              ╱                           ╲
   20k ┤ - - - - - ╱- - - - - - - - - - - - - - - - ╲- - GATE OPEN (valid)
    0  ┤____╱______________________________________╲____
       06:00    09:00    12:00    15:00    18:00    20:00
       ◀── gate closed ──▶◀──── gate open ────▶◀─ closed ─▶
       (night/dawn: any soiling number computed here would be noise)
```

## 4.4 Figure — Alert Timeline of a Dust-and-Clean Event

```
   time ─────────────────────────────────────────────────────────────────▶
   dust      ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│(cleaned)│
   soiling%  0────5────8────13──────18────20%  │   0%    │
                              ▲                │         │
   avg(15)   - - - - - - - - -│- - - - - - - - │- - - - -│- - - -
                              │5 min           │         │
   state     OK ──────────────▶ALERT ─────────▶│OK ──────▶OK
   buzzer    ──────────────── ╱▔▔▔▔▔▔▔▔▔▔▔▔▔▔ ╲______
   LED       ──────────────── ╱▔▔▔▔▔▔▔▔▔▔▔▔▔▔ ╲______
   push      ──────────────── ● (one notification at ON)
   OLED      "OK" …          "CLEANING!"       "OK"
   cloud     state=0 … … …   state=1 … … … …   state=0
                              │                 │
                              └── threshold crossing confirmed by the
                                  5-minute rolling average, then hysteresis
                                  keeps it stable until avg < 8 %
```

## 4.5 Sample Measurement Table (illustrative — replace with your own)

| Day | Condition | I_test (mA) | I_ref (mA) | Ratio | k | Soiling % | Lux | Temp °C | State |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Both clean (calibration) | 500 | 500 | 1.000 | **1.000** | 0.0 | 45 000 | 38 | OK |
| 2 | Light dust | 490 | 500 | 0.980 | 1.000 | 2.0 | 44 500 | 39 | OK |
| 3 | Dust | 475 | 500 | 0.950 | 1.000 | 5.0 | 46 000 | 41 | OK |
| 4 | More dust | 455 | 500 | 0.910 | 1.000 | 9.0 | 43 000 | 42 | OK |
| 5 | Heavy dust | 435 | 500 | 0.870 | 1.000 | 13.0 | 45 500 | 43 | OK |
| 6 | Very heavy | 400 | 500 | 0.800 | 1.000 | **20.0** | 42 000 | 44 | **ALERT** |
| 6 | Cloudy spell | 40 | 50 | 0.800 | 1.000 | *(not evaluated)* | 9 000 | 30 | OK (low light) |
| 7 | Wiped test panel | 498 | 500 | 0.996 | 1.000 | 0.4 | 45 000 | 39 | OK (alert cleared) |

**Reading the table:** from Day 1 to Day 6 the reference stays ~500 mA (sun constant) while the test panel slides — that gap *is* the dust loss. Cloudy readings are deliberately ignored. Cleaning restores the ratio to ≈1.

## 4.6 Blank Data-Log Template (copy into a spreadsheet for every test run)

| Date | Time | Test | Condition (clean/dusty/cloudy/…) | V_test | I_test | V_ref | I_ref | P_test | P_ref | Lux | °C | Soiling % | 5-min avg % | State | Action taken | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|  |  | H/S/W* |  |  |  |  |  |  |  |  |  |  |  |  |  |  |

\* H = hardware accuracy check · S = soiling experiment · W = weather/soak test.
Also keep a **serial log export** (§ 3.11) as raw evidence — copy the timestamped lines into `docs/data/`.

## 4.7 Which Graphs Your Report Needs (minimum set)

| # | Graph | X axis | Y axis | Proves |
| --- | --- | --- | --- | --- |
| 1 | Test vs reference power | time | W | The measurement works and the gap appears |
| 2 | Soiling index over days | date | % | Dust accumulation trend + threshold crossing |
| 3 | Lux vs time (same day) | time | lx | Why low-light readings were rejected |
| 4 | Soiling before/after cleaning | event | % | Cleaning restored output (the "win") |
| 5 | Sensor validation: INA219 vs multimeter | sample # | reading | Hardware accuracy |
| 6 | Alert state timeline | time | 0-3 | Alert logic behaves as designed |

---

# Part V — Workflow & Project Plan

## 5.1 End-to-End Workflow

```
 ┌─────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌───────────┐
 │ 1.BUY   │──▶│ 2.TEST   │──▶│ 3.WIRE   │──▶│ 4.READ   │──▶│ 5.CALIBR- │
 │  parts  │   │  each    │   │  full    │   │  all     │   │  ATE      │
 │         │   │  sensor  │   │  circuit │   │  sensors │   │  (k)      │
 └─────────┘   └──────────┘   └──────────┘   └──────────┘   └─────┬─────┘
                                                                  ▼
 ┌─────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌───────────┐
 │10.DELIVER│◀──│ 9.TUNE   │◀──│ 8.OUT-   │◀──│ 7.Wi-Fi  │◀──│ 6.SOILING │
 │ enclosure│  │ thresholds│  │ DOOR test│   │ + DASH-  │   │ + ALERT   │
 │ report/PPT│  │ & finish │   │ clean vs │   │ BOARD    │   │ LOGIC     │
 │ demo     │  │          │   │ dusty    │   │          │   │           │
 └─────────┘   └──────────┘   └──────────┘   └──────────┘   └───────────┘
```

**Golden rule:** *Test each stage before adding the next* — if stage 4 fails you know it's wiring, not code; if stage 6 fails you know it's logic, not hardware.

## 5.2 Gantt Chart (3-week build, ~3-4 h/day)

```
 Task                       │ W1 Mon Tue Wed Thu Fri │ W2 Mon Tue Wed Thu Fri │ W3 Mon Tue Wed Thu Fri
 ───────────────────────────┼────────────────────────┼────────────────────────┼───────────────────────
 1 Order parts              │ ████                   │                        │
 2 Unbox + blink test       │      ██                │                        │
 3 I2C scanner + sensors    │         ████           │                        │
 4 Bench circuit + verify   │               ████     │                        │
 5 Wokwi / code skeleton    │                  ██    │                        │
 6 Full firmware            │                        │ ██████████             │
 7 Calibration + k storage  │                        │         ████           │
 8 Wi-Fi + ThingSpeak       │                        │               ████     │
 9 Dashboard + notifications│                        │                  ██    │
 10 Outdoor clean-vs-dusty  │                        │                        │ ████
 11 Tune thresholds         │                        │                        │      ██
 12 Frame + enclosure       │                        │                        │         ████
 13 Soak run (days)         │                        │                        │      ▓▓▓▓▓▓▓▓ (parallel)
 14 Report + PPT + demo     │                        │                        │               ████████
 ───────────────────────────┴────────────────────────┴────────────────────────┴───────────────────────
  ██ = working days   ▓▓ = background/soak run
```

## 5.3 Stage Tasks with "Done When" Criteria

| Stage | Tasks | Done when |
| --- | --- | --- |
| 1. Buy | Order BOM (§ 2.2), check panel labels, keep bill | All parts in hand |
| 2. Individual tests | Blink, I2C scan, DS18B20 read, buzzer/LED, relay click | Scanner finds 0x23/0x3C/0x40/0x41 |
| 3. Full wiring | Panels → INA219 → loads; common ground; verify with DMM | H1-H7 tests pass (§ 2.14) |
| 4. Read all sensors | Serial prints of V, I, lux, °C | Values match multimeter/thermometer |
| 5. Calibrate | Both panels clean, full sun, serial `c` | `k` = 0.95-1.05, stored in flash |
| 6. Soiling + alert | Test mode, rolling window, hysteresis, OLED/buzzer | S1-S6 tests pass (§ 3.14) |
| 7. Wi-Fi + dashboard | ThingSpeak channel, 7 fields, charts, notification | Data every 20 s; test alert received on phone |
| 8. Outdoor test | Leave test panel dirty, wipe reference weekly, log data | Clean-vs-dusty graph produced (§ 4.x) |
| 9. Tune | Adjust `LUX_MIN`, 15/8 % thresholds from real data | No false alerts over 3 sunny days |
| 10. Finish | Enclosure, frame, strain relief, report, PPT, demo video | Final integration checklist (§ 6.8) all ticked |

## 5.4 Team Roles (if 2-3 members)

| Member | Owns | Proof of contribution |
| --- | --- | --- |
| A — Hardware | BOM, wiring, frame, enclosure, hardware tests | Wiring doc + photos + H-test table |
| B — Firmware | Drivers, algorithms, alert FSM, calibration | Code repo + S-test results |
| C — Cloud/Report | ThingSpeak/Blynk, dashboard, notifications, data analysis, report/PPT | Dashboard screenshots + graphs + report draft |

(One person can do all three in 3 weeks; split roles for a smoother timeline.)

## 5.5 Deliverables Checklist

- [ ] Working hardware prototype in an enclosure (photo/video evidence)
- [ ] Circuit diagram, wiring table, pin map (§ 2.5-2.8 of this document)
- [ ] Complete, commented firmware (§ 3.9) + repo/backup
- [ ] Dashboard with live link or screenshots (§ 3.13)
- [ ] Test data: clean vs dusty graphs and tables (Part IV)
- [ ] Project report: abstract, introduction, literature survey, block diagram, methodology, results, conclusion (§ 5.6)
- [ ] PPT for presentation
- [ ] Demo video (also useful as a portfolio piece)
- [ ] Viva question prep (Appendix F)

## 5.6 Report & PPT Outline (ready-made structure)

| Report chapter | Content to write (source from this document) |
| --- | --- |
| 1. Abstract | § 0.1 |
| 2. Introduction & problem | § 0.2-0.3, soiling science § 0.7.2 |
| 3. Literature survey | § 0.7.3 comparison table |
| 4. Objectives & scope | § 0.4-0.6 |
| 5. System architecture | Figures 1.2, 1.4, 1.5, 1.8 |
| 6. Hardware design | BOM, pin map, wiring diagrams, mechanical design |
| 7. Software design | Flowcharts 3.5.1-3.5.3, module table, code listing |
| 8. Cloud & dashboard | § 3.12-3.13 with screenshots |
| 9. Results & analysis | Part IV graphs + your measured tables |
| 10. Testing | § 2.14 and § 3.14 result tables |
| 11. Limitations & future scope | § 5.9 + original limitations |
| 12. Conclusion | Achieved objectives, cost, measured savings |

**PPT (12-15 slides):** title → problem → why reference panel → block diagram → hardware photo → wiring → code flowchart → key code → dashboard screenshot → results graphs → testing → limitations → future scope → Q&A.

## 5.7 Risk Register

| Risk | Probability | Impact | Mitigation |
| --- | --- | --- | --- |
| Panel mismatch makes the ratio unreliable | Medium | High | Identical pair from one batch; calibrate `k` while both are clean; verify `k ≈ 1` |
| False alerts on cloudy days / partial shade | High | Medium | Light threshold + rolling average + hysteresis (§ 3.7.2/3.7.5) |
| Sensor noise | Medium | Medium | Average 8 samples; short wires; single ground point |
| Wi-Fi drops | Medium | Low | Non-blocking reconnect every 60 s; local OLED keeps working |
| Water/dust damages electronics | Medium | High | IP54 enclosure, cable glands, desiccant, drip loops |
| Reference panel also gets dirty | High | Medium | Weekly wipe + logged; add a "reference last cleaned" note to the report |
| ThingSpeak free-tier rate limit | Medium | Low | `UPLOAD_MS = 20000` (≥ 15 s) |
| Pump/relay causes ESP32 resets | Medium | Medium | Separate pump supply, common ground, flyback diode, bulk cap |
| Can't get full sun on test day | Medium | Medium | Test mode for logic demos; bright lamp for bench checks; reschedule outdoor run |
| Panel VOC > 26 V (wrong panel bought) | Low | High | Check label **before** buying; INA219 limit is 26 V |
| Last-minute report rush | High | Medium | Keep the data log updated daily; write chapters as stages complete |

**Known limitations (state them honestly in the report):** soiling is estimated indirectly; the prototype is small-scale and not validated on utility arrays; accuracy depends on calibration quality; damp or unevenly deposited dust behaves differently from dry lab soil; the reference panel must be manually kept clean.

## 5.8 Operating & Maintenance Procedure (after the build)

| Frequency | Action |
| --- | --- |
| Continuous | Node logs and uploads; check dashboard when convenient |
| Weekly | **Wipe the reference panel** with a dry cloth; note the date; check enclosure/desiccant |
| Monthly | Verify `k` (both panels clean, run serial `c`); check wires/connectors; clean the BH1750 window |
| After dust storm / heavy soiling | Read soiling %, clean the test panel, confirm the index returns to ~0 % (that's your proof!) |
| On panel change / frame change | Recalibrate `k` |
| Seasonally | Re-check tilt (≈ latitude), shading from growing objects |

## 5.9 Future Scope

- Automatic cleaning with a pump/wiper/brush motor (relay already wired, GPIO27)
- Battery and charge-controller monitoring (add an INA219 on the battery line)
- Machine-learning output prediction and anomaly detection (needs a few months of logged data)
- LoRa or GSM telemetry for sites without Wi-Fi
- Multiple panels/strings behind one gateway (more INA219s on the same I2C bus, 0x40/0x41/0x44/0x45 = 4 per bus, or an I2C multiplexer)
- Weather API integration (dust storms, rain forecasts) → smart, water-saving cleaning schedule
- Solar-powered node (small panel + LiFePO4 + TP4056) for remote installations

---
---

# Part VI — Integration: How Hardware and Software Combine & Work Together

> This is the "put it all together" chapter: how the physical rig, the firmware, the cloud and the human form **one working loop**, how to combine them safely (bring-up order), what happens in a single measurement cycle, and every place the two halves usually fail to talk to each other.

## 6.1 The One-Loop Picture

```
        ┌─────────────────────────────────────────────────────────────────────┐
        │                         THE 20-SECOND LOOP                         │
        │                                                                     │
  SUN   │  ┌──────────┐  I2C/1-Wire ┌──────────┐  numbers  ┌──────────────┐   │
  ─────▶│  │ HARDWARE │────────────▶│ FIRMWARE │──────────▶│  DECISION    │   │
        │  │ panels,  │  V,I,lx,°C  │ ESP32    │  P,soil%  │  alert FSM   │   │
        │  │ sensors  │◀────────────│ (reads,  │◀──────────│  (15%/8%,    │   │
        │  └──────────┘   commands  │ averages,│           │   fault,heat)│   │
        │       ▲                   │ computes)│           └──────┬───────┘   │
        │       │                   └────┬─────┘                  │           │
        │       │                        │                        │           │
        │       │               ┌────────▼────────┐      ┌────────▼────────┐  │
        │       │               │ LOCAL OUTPUT    │      │ Wi-Fi UPLOAD    │  │
        │       │               │ OLED+buzzer+LED │      │ 7 fields / 20 s │  │
        │       │               └────────┬────────┘      └────────┬────────┘  │
        │       │                        │                        │           │
        │       │                        ▼                        ▼           │
        │       │                 on-site human              CLOUD + DASHBOARD│
        │       │                  "go clean it"             graphs · history │
        │       │                                                │             │
        │       └─────── human cleans panels ◀── phone push ─────┘             │
        └─────────────────────────────────────────────────────────────────────┘
                       ▲                                            │
                       └───────────── (optional) relay ──▶ pump ────┘
```

**The loop never stops.** Hardware keeps producing physical signals whether or not Wi-Fi exists; software keeps reading them whether or not the cloud is reachable. Each layer degrades gracefully instead of failing hard.

## 6.2 Layer-by-Layer View (hardware side ↔ software side ↔ what crosses)

| Layer | Hardware side | Software side | Data passed to the next layer |
| --- | --- | --- | --- |
| 1. Sensing | Panels push current through identical load resistors; BH1750 photodiode, DS18B20 junction | *(none — physical signals)* | Continuous analogue quantities: light, heat, current, voltage |
| 2. Acquisition | I2C bus (GPIO21/22) + 1-Wire (GPIO4), 3.3 V rail, common ground | `Wire`, `INA219`, `BH1750`, `DallasTemperature` drivers; 8-sample averaging | Digital values: **V, mA, lx, °C** |
| 3. Processing | ESP32 CPU (240 MHz) | `readSensors()`, `computeSoiling()`: P = V×I, validity gate, `soil = (1−(I_t/I_r)/k)×100`, rolling window | **P (W), soiling %, 5-min mean, validity flag** |
| 4. Decision | ESP32 CPU | `updateAlert()`: fault check → overheat check → hysteresis thresholds on the mean | **`state` (OK/ALERT/FAULT/OVERHEAT) + `alarmActive`** |
| 5. Local output | OLED RAM/panel, buzzer, LED, (relay) | `updateOLED()`, `updateOutputs()`, (relay driver) | Visible + audible alert that works **offline** |
| 6. Communication | ESP32 Wi-Fi radio → router → internet | `wifiEnsure()`, `upload()`: HTTP POST of 7 fields every 20 s | **ThingSpeak record with timestamp** |
| 7. Cloud & user | Server, phone, PC | Dashboard widgets, charts, React/Blynk notification rules | **Graphs, history, and the "clean your panel" message to a human** |
| 8. Feedback | Human cleans test panel / pump sprays | *(state returns to OK; the loop restarts)* | Physical change measured again from layer 1 |

**Protocol summary:** layers 1→2 = physical; 2→3 = `Wire`/`OneWire` digital reads; 3→4→5 = in-memory function calls; 6→7 = Wi-Fi + HTTP; 7→8 = human (or relay→pump).

## 6.3 Interface Contract (who produces what, who consumes it)

| Signal | Produced by (HW or SW) | Consumed by | Format / unit | Rate | Failure behaviour |
| --- | --- | --- | --- | --- | --- |
| Panel current test/ref | HW (INA219) | SW averaging | mA, float | 8 samples/5 s | Missing → FAULT or error print, keep running |
| Bus voltage test/ref | HW (INA219) | SW | V, float | 5 s | — |
| Light level | HW (BH1750) | SW validity gate | lx, float | 5 s | 0/65535 → gate blocks evaluation |
| Panel temperature | HW (DS18B20) | SW alert engine | °C, float | 5 s | −127 → treated as 0, warn in log |
| Power | SW (`P=V×I`) | SW, OLED, cloud | W, float | 5 s | — |
| Soiling % | SW (ratio vs `k`) | SW window, OLED, cloud | %, float | 5 s | Invalid → not computed, window untouched |
| 5-min mean | SW (window) | Alert engine, cloud field 6 | % | 5 s | Window reset if stale >10 min |
| `state` code | SW (FSM) | Buzzer/LED/OLED, cloud field 7 | 0-3 int | 5 s | — |
| Alarm drive | SW | HW buzzer/LED/relay | GPIO HIGH/LOW | continuous | — |
| 7-field update | SW | Cloud | HTTP POST | 20 s | Skipped if offline; retried next tick |
| Dashboard/notification | Cloud | Human | Web page / push | live | Human cleans → HW loop restarts |

## 6.4 Bring-Up Order: How To Combine Them (do in this exact order)

| # | Combine | Do this | Pass test | If it fails |
| --- | --- | --- | --- | --- |
| 1 | **Hardware only** | Wire everything, power the ESP32, run I2C scanner | Finds **0x23, 0x3C, 0x40, 0x41**; DS18B20 responds | § 2.15 (wiring, ground, addresses) |
| 2 | **Hardware → software (raw)** | Upload firmware, watch Serial Monitor | Every sensor prints plausible values; matches multimeter/thermometer | § 3.16; test one sensor at a time |
| 3 | **Calibration** | Both panels clean, full sun, serial `c` | `k` = 0.95-1.05, stored, survives reboot | Bad light, dirty panel, or mismatched panels |
| 4 | **Processing** | Verify P, soiling %, light gate on Serial + OLED | Soiling ≈ 0 % with clean panels; gate blocks at night | § 3.7 math, `k`, `valid` flag |
| 5 | **Decision** | Force a case: test mode `t`, or part-cover the test panel | After 5 min: buzzer + LED + OLED "CLEANING!" | Window/thresholds (§ 3.8) |
| 6 | **Communication** | Wi-Fi credentials, ThingSpeak channel, upload | All 7 fields arrive every 20 s | Wrong key/ID, 5 GHz network, <15 s interval |
| 7 | **Cloud → user** | Add widgets + React/Blynk notification | Phone/e-mail alert arrives when state = 1 | Notification rule config |
| 8 | **Full system outdoors** | Enclose, run 3-7 days, clean-vs-dusty test | Trend graph + alert + recovery after cleaning | Reference panel hygiene, shading |

> **Rule: never skip a row.** If row 7 fails after you jumped from row 3, you can't tell whether it's wiring, math, or credentials. One layer at a time = every fault has exactly one suspect.

## 6.5 One Measurement Cycle (timings are approximate)

```
  0 s        5 s       10 s      15 s       20 s       25 s
   │──────────│──────────│──────────│──────────│──────────▶
   ├ READ (5 s tick)                                          
   │  sense → average → compute → decide → show              
   ├──────────────────────── UPLOAD (20 s tick) ─────────────┤
   │                    Wi-Fi POST (1-3 s)                    │
   ├──────────────────────────────────────────────────────────┤
   │  local outputs updated every loop (beep, LED)            │
```

| Step | Approx. time | Hardware does | Software does |
| --- | --- | --- | --- |
| 1. Sense | 0 ms | Sun drives current through both panels; INA219 shunts, BH1750 photodiode, DS18B20 junction respond | Nothing (physical) |
| 2. Read | ~100-300 ms | I2C + 1-Wire carry bytes to the ESP32 | `readAveraged()` takes 8 samples per INA219 and averages; lux + °C read |
| 3. Compute | <1 ms | CPU executes | P = V×I both panels; validity gate; soiling % vs `k`; push to 15-sample window |
| 4. Decide | <1 ms | Nothing | Fault → overheat → hysteresis on the 5-min mean → `state` |
| 5. Act locally | ~10-50 ms | OLED RAM updated, buzzer/LED pins set | `updateOLED()`, `updateOutputs()` (beep pattern runs every loop) |
| 6. Send (every 20 s) | 1-3 s | Radio → router → internet | `upload()`: HTTP POST of 7 fields (skipped if offline) |
| 7. Show | a few seconds | Cloud stores; phone/PC fetches | Charts update; notification if `state` = 1 |
| 8. Wait | rest of the cycle | Idle, sensors still physically responding | Non-blocking `millis()` scheduling — loop keeps running |
| **Full cycle** | **20 s** | — | Next read at 5 s ticks, next upload at 20 s ticks |

## 6.6 Worked Example: Dust Building Up Over One Week

*Numbers assume `k = 1.00`, sunny conditions, `LUX_MIN = 20 000`.*

| When | I_test / I_ref | Soiling % | 5-min mean | What hardware shows | What software does | What the user sees |
| --- | --- | --- | --- | --- | --- | --- |
| Day 1, clean (after `c`) | 500/500 = 1.00 | 0 % | 0 % | Both panels identical output | `k` stored; state = OK | Dashboard flat at 0 %, "OK" |
| Day 4, light dust | 460/500 = 0.92 | 8 % | ~5 % | Test panel slightly dimmer | Below alert threshold, window filling | Rising line on the graph, still "OK" |
| Day 6, heavy dust | 400/500 = 0.80 | 20 % | ≥ 15 % for 5 min | Clear visible dust layer | Mean crosses 15 % with full window → **state = 1** | OLED "CLEANING!", buzzer, LED, phone push, red gauge |
| Day 7, after wiping | 498/500 ≈ 1.00 | 0.4 % | < 8 % | Test panel shiny again | Mean < 8 % → state = 0, alarm drops | Alert clears; "recovered ~19 % output" |
| Any cloudy afternoon | 40/50 (both weak) | *(not evaluated)* | held | Both panels output little | `valid = false` (lux < 20 000) → no computation, no state change | "LOW LIGHT" on OLED, **no false alarm** |
| Any morning before ~8 am | — | *(not evaluated)* | held | Too little light for a stable ratio | Same gate | No false alarm |
| Day 6 wire comes loose | 0/500 | *(n/a)* | — | Test panel line open | `I_ref > 50 mA && I_test < 5 mA` → **state = 2** | "FAULT!" — distinguishes *broken* from *dirty* |

**Sequence in prose (the story your viva should tell):** *sun → both panels → shunts → I2C → ESP32 averages → ratio vs k → gated by lux → 5-minute mean → hysteresis decision → OLED/buzzer AND (every 20 s) Wi-Fi → cloud → dashboard → phone push → human cleans → ratio returns to 1 → alert clears.*

## 6.7 Common Integration Problems & Fixes

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| I2C scan finds no devices | Wrong SDA/SCL pins, no common GND, no 3.3 V | Recheck § 2.7; share one ground; measure 3.3 V |
| One INA219 missing, or both read identically | Both at address 0x40 | Bridge A0 → VCC on the second module (0x41) |
| Current reads zero or negative | Shunt wired backwards | Swap VIN+ and VIN− |
| DS18B20 reads −127 °C | Missing 4.7 kΩ pull-up / loose data wire | Fit pull-up between DATA and 3.3 V |
| Soiling index jumps around | Noise, partial shade, or low light | Raise `N_SAMPLES`; keep the light gate + rolling average |
| Soiling stays ~0 % despite visible dust | Reference panel is **also dirty** | Wipe the reference panel (this is the #1 field failure) |
| `k` far from 1.0 | Panels mismatched, different tilt, or one was dirty at calibration | Fix mounting, verify cleanliness, re-run `c` |
| Alert fires at night/cloud | Light gate too low or broken | Check `lux` value; `LUX_MIN` must exceed dawn/dusk readings |
| Alert never clears | Window holds stale data, or reference dirty | Check `STALE_MS` reset; wipe reference; press `r` to reset window |
| No data on dashboard | Wrong channel ID/key, 5 GHz network, uploads < 15 s | Check credentials; use a **2.4 GHz** SSID; keep 20 s |
| Data arrives but charts look wrong | Fields in the wrong order | Match the field map in § 3.12 exactly |
| ESP32 resets when the pump starts | Pump shares the ESP32's supply | Separate pump supply, common GND, 1N4007 flyback, bulk cap |
| Works on the bench, fails outdoors | Heat, moisture, loose jumpers | Enclosure + glands + perfboard; re-run the I2C scan outdoors |
| Readings differ from the multimeter | Shunt tolerance, library calibration range | Cross-check per § 2.14 (H3/H4); note accuracy in the report |

## 6.8 Final Integration Checklist

**Hardware**
- [ ] I2C scan shows 0x23, 0x3C, 0x40, 0x41
- [ ] All readings match the multimeter/thermometer within a few percent
- [ ] Both panels identical model, same tilt/azimuth, no mutual shading
- [ ] DS18B20 taped to the test panel's back; BH1750 flat with sky view
- [ ] Enclosure weatherproofed, strain-relieved, desiccant fitted

**Software**
- [ ] `k` stored in flash and close to 1.0 (0.95-1.05)
- [ ] OLED shows live values + status; low-light state shown without alarms
- [ ] Test mode reproduces 20 % soiling and triggers the alert after 5 minutes
- [ ] Alert triggers on demand and clears only below 8 %
- [ ] Fault (disconnected test panel) and overheat (T > 65 °C) states verified

**Cloud & user**
- [ ] All 7 fields reach the dashboard every 20 s
- [ ] Graphs for power (test vs ref) and soiling % visible with history
- [ ] Phone/e-mail notification received on a real alert event

**System**
- [ ] Runs 24 hours unattended with no resets
- [ ] Wi-Fi drop recovers by itself within ~60 s
- [ ] Clean-vs-dusty outdoor test recorded (table + graph from Part IV)
- [ ] Cleaning event: alert → clean → alert clears → output recovery documented

## 6.9 The 60-Second Plain-English Explanation (for the viva/demo)

1. **The hardware** is two identical small solar panels on one frame — one we keep clean, one we let get dusty — each pushing current through the same-size resistor, with a chip (INA219) on each line measuring exactly how much voltage and current it produces. A light sensor tells us how strong the sun is, and a temperature sensor tells us how hot the panel gets. All of that is wired to an ESP32, a small Wi-Fi computer.
2. **The software** on the ESP32 reads those sensors five times a second-ish, averages out noise, multiplies voltage by current to get power, and then compares the dusty panel's current with the clean panel's current. Because both panels see the same sun and the same heat, the only reason one is weaker is dust. A calibration constant `k`, measured once when both were spotless, removes any factory difference between them. The result is a **Soiling Index in percent**.
3. **We refuse to trust that number in bad conditions**: if the light sensor says it's cloudy or early morning, we simply skip the calculation — that's what stops false alarms. We then average the last five minutes of readings, and only if the average stays above **15 %** do we raise **"Cleaning Required"**; it only clears when it falls below **8 %**, so it can't flicker on and off.
4. **The user gets the message three ways**: an OLED screen and buzzer right at the panel (works even without internet), and — every 20 seconds — the numbers are pushed over Wi-Fi to a cloud dashboard where you see live gauges and graphs, plus a phone notification when cleaning is due.
5. **After cleaning**, the dusty panel's current comes back up to the reference's, the index falls below 8 %, the alert clears, and the dashboard shows exactly how many percent of energy you just recovered. An optional relay can do the cleaning automatically.

## 6.10 Quick-Start Card (if you're building this tomorrow)

```
  DAY 1        order BOM (§2.2) · Arduino IDE + libraries (§3.2) · blink + I2C scanner (App. B)
  DAY 2-3      wire per tables §2.7-2.8 · hardware tests §2.14 (H1-H6)
  DAY 4-5      upload §3.9 firmware · verify Serial values §3.11 (test S1-S6)
  DAY 6        both panels clean + strong sun → serial 'c' → record k
  DAY 7        ThingSpeak channel + keys + charts (§3.12-3.13) → live data
  DAY 8-14     outdoor run: leave test panel dirty, wipe reference weekly, log data (§4.6)
  DAY 15-16    tune thresholds from real data (§3.8) · verify alert end-to-end
  DAY 17-19    enclosure + frame photos · collect graphs (§4.7)
  DAY 20-21    report (§5.6) · PPT · demo video · viva prep (App. F)
```

---
---

# Appendices

## Appendix A — Formulas & Calculations (quick reference)

| # | Formula | Symbols | Example |
| --- | --- | --- | --- |
| A1 | `P = V × I` | V volts, I amps | 6.0 V × 0.486 A = 2.92 W |
| A2 | `k = I_test / I_ref` (both clean) | calibration constant | 500/500 = 1.000 |
| A3 | `Soiling % = (1 − (I_test/I_ref)/k) × 100` | — | (1 − 0.80/1.00)×100 = 20 % |
| A4 | `R_load = Vmp / Imp` | panel MPP values | 6 V / 0.5 A = 12 Ω (≥ 4.5 W) |
| A5 | `P_resistor ≈ P_panel` at that R | choose ≥ 1.5× rating | 3 W panel → ≥ 5 W resistor (10 W preferred) |
| A6 | `I = V_shunt / 0.1 Ω` | INA219 shunt | 48.6 mV / 0.1 = 0.486 A |
| A7 | `ΔP_temp ≈ P × 0.004 × (T − 25 °C)` | temp. coefficient ~0.4 %/°C | 3 W at 65 °C → ≈ 0.48 W lost to heat |
| A8 | `Rolling mean = Σ(last 15 valid samples)/15` | 15 × 20 s = 5 min | — |
| A9 | `Alert ON: mean > 15 %` · `OFF: mean < 8 %` | hysteresis band = 7 % | — |
| A10 | Daily energy `Wh = Σ(P × Δt)` with Δt = 20 s | for the dashboard | 2.5 W × 24 h ≈ 60 Wh/day (small panel) |
| A11 | Lux gate: valid if `lux > 20 000` | — | full sun ≈ 50 000-100 000 lx |
| A12 | Fault: `I_ref > 50 mA AND I_test < 5 mA` | — | panel dead, sun fine |

**Unit conversions:** mA → A divide by 1000 · W → mW ×1000 · °F = °C×1.8+32 · panel % loss × W ≈ watts you're losing.

## Appendix B — I2C Scanner Sketch (run first, always)

```cpp
#include <Wire.h>
void setup() {
  Wire.begin(21, 22);              // SDA = GPIO21, SCL = GPIO22
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("\nI2C scanner running...");
}
void loop() {
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X  <--- device found\n", addr);
      found++;
    }
  }
  Serial.printf("Total: %d device(s). Expected: 0x23, 0x3C, 0x40, 0x41\n\n", found);
  delay(5000);
}
```

## Appendix C — Log & CSV Format (for graphs in Excel/Sheets)

**Serial line (from `printStatus()`):**
```
[120] VT=6.01 IT=486 VR=6.03 IR=494 P=2.92 lux=42150 T=41.6 soil=1.6 avg=1.4 valid=1 state=0 wifi=1
```

**CSV columns for `docs/data/run1.csv`:**
```csv
timestamp,v_test,i_test,v_ref,i_ref,p_test,p_ref,lux,temp_c,soiling_pct,avg_pct,state,wifi
2026-03-01 10:20:00,6.01,486,6.03,494,2.92,2.98,42150,41.6,1.6,1.4,0,1
```
Capture it with the Arduino IDE Serial Monitor → "Save output", or `pio device monitor | tee log.txt`, then import the CSV and plot § 4.7's six graphs.

## Appendix D — Glossary (plain-language)

| Term | Meaning |
| --- | --- |
| Soiling | Dirt/dust/pollen layer on a panel that blocks sunlight |
| Reference panel | The clean panel used as a fair baseline for comparison |
| Test panel | The panel deliberately left uncleaned |
| Soiling index | Percent of output lost to dust, 0 % = perfectly clean |
| Calibration constant `k` | Ratio of the two panels' currents when both are clean |
| Hysteresis | Using two thresholds (15 % / 8 %) so an alert can't flicker |
| Rolling average | Mean of the last N readings (here 15 = 5 minutes) |
| Validity gate | A check (light > 20 000 lx) that decides whether to trust a reading |
| I2C | Two-wire digital bus (SDA/SCL) shared by up to 127 devices |
| 1-Wire | Maxim's single-data-wire bus used by DS18B20 |
| Shunt | Precision resistor (0.1 Ω) across which voltage is measured to get current |
| INA219 | Chip measuring bus voltage + shunt current, reports over I2C |
| BH1750 | Digital light (lux) sensor |
| Voc / Vmp / Imp | Open-circuit voltage / voltage & current at max power |
| MPP | Maximum power point — the panel's best operating point |
| HMI | Human-machine interface (here: OLED + dashboard) |
| Telemetry | Measuring remotely and sending the data (here over Wi-Fi) |
| Hysteresis band | The gap between ON (15 %) and OFF (8 %) thresholds |
| Non-blocking code | Code that never sits in `delay()`; uses timers instead |
| NVS / Preferences | ESP32's flash key-value storage (survives power-off) |
| ThingSpeak | MathWorks' free IoT cloud for charts and alerts |
| Flyback diode | Diode protecting circuits from a pump/relay's voltage spike |

## Appendix E — References & Datasheets (verify links before submission)

| Component | Document to cite |
| --- | --- |
| ESP32 | Espressif, *ESP32 Datasheet* — https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf · *ESP32 Hardware Design Guidelines* |
| INA219 | Texas Instruments, *INA219 Zero-Drift Bi-Directional Current/Power Monitor* — https://www.ti.com/lit/ds/symlink/ina219.pdf |
| BH1750 | ROHM, *BH1750FVI Digital Light Sensor Datasheet* |
| DS18B20 | Analog Devices (Maxim), *DS18B20 Programmable Resolution 1-Wire Digital Thermometer* — https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf |
| SSD1306 | Solomon Systech, *SSD1306 128×64 Dot Matrix OLED Controller/Driver* |
| Libraries | Adafruit INA219, claws/BH1750, OneWire (P. Stoffregen), DallasTemperature, Adafruit SSD1306/GFX, MathWorks ThingSpeak — all via the Arduino Library Manager |
| Method | IEC 61724-1 (PV system performance monitoring) — background reading for soiling/reference-cell practice |
| Cloud | ThingSpeak documentation & React/ThingHTTP guides; Blynk documentation |

## Appendix F — Likely Viva Questions (with short answers)

1. **Why INA219 instead of ACS712?** It measures voltage *and* current digitally over I2C with good resolution at low currents; ACS712's analogue output is noisy at the ~0.5 A scale of small panels.
2. **Why two panels?** The clean reference gives a baseline under the *same* sunlight, so the difference is caused by dust, not by weather.
3. **Why not a dust sensor?** Optical dust sensors measure airborne particles, not the layer stuck on the panel surface — a poor proxy for actual power loss.
4. **How do you avoid false alerts on cloudy days?** BH1750 light threshold (>20 000 lx), a 5-minute rolling average, and 15 %/8 % hysteresis.
5. **What is `k`?** The ratio of the two panels' currents measured when both are spotless; it corrects for manufacturing differences. Expect 0.95-1.05.
6. **Why is temperature not in the soiling formula?** Both panels are side by side at nearly the same temperature, so the ratio cancels the ~0.4 %/°C effect; temperature is used for the overheating warning and context.
7. **Why 20 seconds between uploads?** ThingSpeak's free tier allows updates no faster than 15 s; soiling changes slowly, so 20 s is plenty.
8. **What happens if Wi-Fi goes down?** The node keeps measuring, shows OLED/buzzer locally, retries every 60 s, and resumes uploads automatically — no reboot.
9. **Why hysteresis?** A single 15 % threshold would make the alarm flicker around the boundary; two thresholds give one clean transition.
10. **How do you calibrate?** Both panels clean, strong sun, stable readings → serial command `c` → `k = I_test/I_ref` stored in flash.
11. **How do you prove it works?** Sprinkle soil/flour on the test panel → index rises → alert fires; wipe it → index returns to ~0 % → alert clears; covered/cloudy → no evaluation, no false alarm.
12. **What is the accuracy?** Quote your own validation: INA219 vs multimeter (±2-5 %), DS18B20 ±0.5 °C, lux used only as a threshold (BH1750 saturates above ~65 k lx).
13. **What does FAULT mean vs ALERT?** FAULT = reference producing but test panel dead (wiring/panel problem); ALERT = sustained dust loss. Different causes, different responses.
14. **Why ESP32 over Arduino/ESP8266?** Built-in 2.4 GHz Wi-Fi plus enough GPIO/I2C pins to run four I2C devices and 1-Wire on one board, with a large library ecosystem.
15. **Why I2C for four devices?** Only two wires shared by all devices; no address conflicts (0x23, 0x3C, 0x40, 0x41).
16. **Why a reference-panel method rather than an I-V curve tracer?** Cost and continuous automatic operation; a curve tracer needs manual, disconnecting measurements.
17. **What are the limitations?** Indirect estimate; prototype-scale; depends on calibration and on manually keeping the reference clean; damp/uneven dust behaves differently from lab soil.
18. **How does it scale?** More INA219 pairs per ESP32 (addresses 0x40/0x41/0x44/0x45), multiple nodes → one dashboard; LoRa/GSM for sites without Wi-Fi.
19. **What is the cost?** ~Rs 3,000-4,500 for the recommended college version; ~Rs 4,500-7,000 with auto-cleaning.
20. **What would you improve with more time?** Automatic cleaning, ML-based anomaly detection, solar-powered node, multi-panel strings, weather-API-aware scheduling.

## Appendix G — Freelance Pricing (estimates, not verified rates)

Market estimates for a beginner freelancer serving B.Tech students in India — adjust to your market.

| Package | Without hardware (code, diagram, dashboard, documentation) | With hardware (built and delivered) |
| --- | --- | --- |
| Basic (sensors, ESP32, dashboard) | Rs 3,000-5,000 | Rs 6,000-8,000 |
| **Recommended (+ soiling logic, alerts, report)** | **Rs 5,000-7,000** | **Rs 8,000-12,000** |
| Advanced (+ auto-cleaning pump) | Rs 7,000-10,000 | Rs 12,000-18,000 |

- With hardware: price at parts cost **plus 2-3× that** for your time.
- Charge extra for the report, PPT, viva preparation and revisions.
- Take a 50 % advance, balance on delivery.
- Include a walkthrough session so the student can explain and demo their own project — a client who can't explain their project fails the viva, and that damages your reputation.

## Appendix H — What's New vs. the Original Brief

| Original brief (v1.0) | This document (v2.0) |
| --- | --- |
| 16 sections, single flow | Re-organised into **Foundation → System Design → Hardware → Software → Charts → Workflow → Integration**, with the integration chapter kept as the **last** part as requested |
| One block diagram, one flowchart | **25+ figures**: context, block, layer stack, I2C topology, power tree, schematics, flowcharts, state machine, timing, Gantt, dashboards, 6 data charts |
| Hardware section with pin tables | Full BOM + deep-dive per component + **master wiring table (every wire)** + assembly procedure + hardware test/troubleshooting tables + mechanical & enclosure design |
| Skeleton code (~70 lines) | **Complete firmware** with rolling window, staleness reset, FSM, calibration persistence, non-blocking Wi-Fi, OLED UI, serial CLI, test mode + module inventory and tuning table |
| Brief dashboard/ThingSpeak notes | Step-by-step cloud setup, **field map**, dashboard mockup, notification options, platform comparison |
| Testing tables | Two acceptance suites (hardware + software), 6 report-ready graphs, blank data-log template |
| Integration chapter | **Expanded**: interface contract table, layer data-crossing table, 8-step bring-up order with pass/fail, per-second cycle timing, worked weekly example, plain-English 60-second explanation, quick-start card |
| — | New appendices: formulas, I2C scanner, CSV format, glossary, datasheet references, 20 viva Q&A |

---

**End of document.** With this file alone — BOM, wiring, firmware, cloud setup, test plan and integration procedure — you can build, test, demo and defend the project without researching from scratch.

