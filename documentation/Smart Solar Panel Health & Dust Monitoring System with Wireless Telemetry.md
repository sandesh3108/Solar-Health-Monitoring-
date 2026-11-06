# Smart Solar Panel Health & Dust Monitoring System with Wireless Telemetry

Project Documentation | IoT / Embedded Systems | Final-Year B.Tech

## 1. Project Overview

This project is an IoT-based monitoring system that continuously measures a solar panel's voltage, current, power, temperature and sunlight, estimates how much output is being lost to dust (soiling), and sends everything over Wi-Fi to a mobile/web dashboard. When performance drops abnormally, it raises a "Panel Cleaning Required" alert.

**Problem:** Dust on panels can quietly cut energy output by 5-30% depending on climate and cleaning frequency. Most small installations have no way to know when cleaning is actually worth doing, so panels are cleaned too late or on a blind schedule.

**Solution:** Compare a dusty (test) panel against a clean reference panel under the same sunlight, compute a soiling index, and alert only when the loss is real, not just because it is cloudy.

## 2. Objectives

- Measure panel voltage and current, and calculate power (P = V x I)
- Measure sunlight intensity (lux) and panel/ambient temperature
- Estimate soiling loss using a clean reference panel
- Detect abnormal output drops after correcting for light and temperature
- Transmit readings wirelessly over Wi-Fi every 15-20 seconds
- Show live and historical data on a dashboard
- Generate a cleaning/maintenance alert (buzzer + dashboard + phone notification)
- Optional: trigger an automatic cleaning pump

## 3. System Architecture

### 3.1 Block Diagram

```
+-------------+    +-------------------+
| TEST PANEL  |--->| INA219 #1 (0x40)  |--+
| (dusty)     |    | V + I             |  |
+-------------+    +-------------------+  |
                                          |
+-------------+    +-------------------+  |   I2C bus     +------------------+
| REFERENCE   |--->| INA219 #2 (0x41)  |--+-------------->|                  |
| PANEL(clean)|    | V + I             |  |   (SDA/SCL)   |                  |
+-------------+    +-------------------+  |               |      ESP32       |
                                          |               |  - read sensors  |
                   +-------------------+  |               |  - compute P,    |
                   | BH1750 (0x23)     |--+               |    soiling index |
                   | Light (lux)       |  |               |  - alert logic   |
                   +-------------------+  |               |  - Wi-Fi upload  |
                                          |               |                  |
                   +-------------------+  |               +---+----+----+----+
                   | OLED SSD1306(0x3C)|<-+                   |    |    |
                   +-------------------+                      |    |    |
                                                              |    |    |
   +-------------------+   1-Wire (GPIO4)                     |    |    |
   | DS18B20 Temp      |------------------------------------->|    |    |
   +-------------------+                                           |    |
                                                 Buzzer/LED <------+    |
                                                 Relay -> Pump (opt.) <-+

                                   Wi-Fi
                      ESP32 ------------------> Cloud (ThingSpeak / Blynk / Firebase)
                                                         |
                                                         v
                                              Mobile / Web Dashboard + Alerts
```

### 3.2 Data Flow

```
Sensors -> ESP32 (sample + average) -> Compute (P, soiling %, temp-corrected loss)
        -> Alert logic -> Local output (OLED, buzzer) + Wi-Fi upload -> Dashboard
```

### 3.3 How Soiling Is Estimated

An optical dust sensor (such as GP2Y1010) measures dust floating in the air, not dust sitting on the panel, so it is a weak proxy. This project uses the **reference-panel method** instead:

1. Use two identical small panels mounted side by side at the same tilt and direction.
2. Keep the **reference panel clean** (wipe it regularly). Leave the **test panel** uncleaned.
3. Connect both to identical fixed loads and measure the current of each with its own INA219. Current from a panel is roughly proportional to sunlight, so a dirty panel produces less current than the clean one under the same light.
4. **Calibrate once** when both panels are clean: k = I\_test / I\_ref (this absorbs small manufacturing differences).
5. At run time, the soiling index is:

```
Soiling Index (%) = ( 1 - (I_test / I_ref) / k ) x 100
```

The BH1750 light reading is used as a validity check: the index is only computed when sunlight is strong enough to give a stable reading (see Section 6).

## 4. Components and Bill of Materials

| # | Component | Purpose | Qty | Approx. cost (INR) |
| --- | --- | --- | --- | --: |
| 1 | ESP32 DevKit (30/38 pin) | Controller + Wi-Fi | 1 | 430-500 |
| 2 | Solar panel, identical pair (e.g. 6 V / 2-3 W) | Test + reference panel | 2 | 500-1,000 |
| 3 | INA219 module | Voltage + current (I2C) | 2 | 250-400 |
| 4 | BH1750 module | Light intensity (lux) | 1 | 70-150 |
| 5 | DS18B20 (waterproof probe) + 4.7 kOhm resistor | Panel temperature | 1 | 80-150 |
| 6 | 0.96" OLED SSD1306 (I2C) | Local display | 1 | 120-250 |
| 7 | Buzzer + LED + resistor | Local alert | 1 | 30-80 |
| 8 | Fixed load resistors (same value for both panels, rated for panel power) | Identical load on both panels | 2 | 20-60 |
| 9 | Breadboard / perfboard + jumper wires | Connections | 1 | 150-300 |
| 10 | 5 V supply (USB adapter or buck converter) | Power the ESP32 | 1 | 100-300 |
| 11 | Enclosure + mounting frame | Protection + fixed tilt | 1 | 200-500 |
| 12 | Optional: relay module + mini pump + tubing | Auto-cleaning | 1 set | 600-1,500 |

**Estimated total:** basic prototype Rs 2,000-3,000; recommended college version Rs 3,000-4,500; with automatic cleaning Rs 4,500-7,000. These are estimates, so check current prices before buying.

## 5. Hardware Part

### 5.1 ESP32 Pin Mapping

| Function | ESP32 pin | Notes |
| --- | --- | --- |
| I2C SDA | GPIO21 | Shared by INA219 x2, BH1750, OLED |
| I2C SCL | GPIO22 | Shared by all I2C devices |
| DS18B20 data | GPIO4 | 4.7 kOhm pull-up to 3.3 V |
| Buzzer | GPIO25 | Active buzzer through a transistor if needed |
| Status LED | GPIO26 | Series resistor 220-330 Ohm |
| Relay IN (optional) | GPIO27 | For pump control |

### 5.2 I2C Addresses

| Device | Address |
| --- | --- |
| INA219 #1 (test panel) | 0x40 (default) |
| INA219 #2 (reference panel) | 0x41 (bridge the A0 pad to VCC) |
| BH1750 | 0x23 (ADDR pin low) |
| OLED SSD1306 | 0x3C |

### 5.3 Wiring Notes

- Power all I2C modules from the ESP32 3.3 V pin and share a common GND.
- Each INA219 connects in series with its panel's positive line: panel+ -> VIN+ -> VIN- -> load -> panel-. Panel negative returns to common ground.
- The INA219 bus voltage range is up to 26 V and the standard 0.1 Ohm shunt measures up to about 3.2 A, which is fine for small panels. Do not connect a panel whose open-circuit voltage exceeds 26 V.
- Use the same wire gauge and length on both panel lines so the comparison stays fair.
- Mount the DS18B20 probe on the back of the test panel with thermal tape.
- Never power the ESP32 directly from the panel. Use a regulated 5 V source.

### 5.4 Mechanical Setup

- Mount both panels on one frame, same tilt (roughly equal to local latitude), same direction, no shading from each other.
- Place the BH1750 flat next to the panels with a clear view of the sky.
- Keep the electronics in a ventilated, rain-protected enclosure.

### 5.5 Component Details

| Component | What it does | How it is used here | Key facts |
| --- | --- | --- | --- |
| ESP32 DevKit | Runs the firmware, reads sensors, sends data over Wi-Fi | Central controller on one I2C bus plus one 1-Wire pin | 3.3 V logic, built-in 2.4 GHz Wi-Fi |
| Solar panels (test + reference) | Turn sunlight into DC power | Identical pair; test panel stays uncleaned, reference stays clean | Open-circuit voltage must stay under 26 V (INA219 limit) |
| INA219 x 2 | Measures bus voltage and, through a 0.1 Ohm shunt, current | One per panel, in series with the panel's positive line | 0-26 V, up to about 3.2 A, I2C |
| BH1750 | Measures light intensity in lux | Validity check for the soiling calculation | I2C; default range tops out near 65,535 lx, so full sun can saturate it, which is fine for a threshold check |
| DS18B20 | Measures temperature digitally | Taped to the back of the test panel | 1-Wire, about +/-0.5 C, needs a 4.7 kOhm pull-up |
| OLED SSD1306 | Shows live values and status on the device | Local readout that works without Wi-Fi | 128x64 pixels, I2C |
| Buzzer + LED | Local alert | Switched on when the alert flag is set | Use a transistor if the buzzer draws more than a GPIO can supply |
| Relay + pump (optional) | Switches the cleaning pump | Fires on a cleaning command | Separate supply, flyback diode |

### 5.6 Power Supply and Loads

- Power the ESP32 from a regulated 5 V USB adapter rated at least 1 A, because Wi-Fi transmission causes current spikes.
- The panels only drive the load resistors being measured. They do not power the electronics.
- Pick the load resistor from the panel label: R is roughly Vmp / Imp (a 6 V, 0.5 A panel gives about 12 Ohm), rated at least 5 W. Check your own panel's label, and use the same value on both panels.
- Optional pump: separate supply, common ground with the ESP32, and a flyback diode across the pump.

### 5.7 Hardware Assembly and Check

1. Test the ESP32 alone: upload a blink sketch and print to the Serial Monitor.
2. Add the I2C devices one at a time and run an I2C scanner. Expect 0x23, 0x3C, 0x40 and 0x41.
3. Add the DS18B20 and compare its reading with a normal thermometer.
4. Connect each panel through its INA219 to its load resistor and compare voltage and current with a multimeter.
5. Mount both panels on the frame at the same tilt, add strain relief to the wires, and place the electronics in the enclosure.

**Hardware is done when:** all four I2C addresses appear, every reading matches the multimeter or thermometer within a few percent, and the wiring survives being moved.

## 6. Software Part

### 6.1 Tools and Libraries

- Arduino IDE or PlatformIO
- Libraries: Adafruit INA219, BH1750, OneWire, DallasTemperature, Adafruit SSD1306 + Adafruit GFX, WiFi, ThingSpeak (or Blynk / Firebase client)
- Dashboard: ThingSpeak (easiest to start), Blynk (best mobile app and notifications) or Firebase + a custom web page
- Wokwi for simulation before wiring real hardware

### 6.2 Program Flow

```
START
  |
  v
Initialize I2C, sensors, OLED, Wi-Fi
  |
  v
Calibration done? --No--> Run clean-panel calibration, store k in flash
  |Yes
  v
+---------------------- LOOP (every 15-20 s) ----------------------+
| 1. Read V, I from both INA219; read lux; read temperature        |
| 2. Average several samples to reduce noise                       |
| 3. P_test = V x I (test), P_ref = V x I (reference)              |
| 4. If lux > threshold: Soiling Index = 1 - (I_test/I_ref)/k      |
|    else: mark reading as "low light, ignored"                    |
| 5. Track a rolling average of the soiling index                  |
| 6. Alert logic (Section 6.3)                                     |
| 7. Update OLED + buzzer/LED                                      |
| 8. Upload to cloud over Wi-Fi (retry on failure)                 |
+-------------------------------------------------------------------+
```

### 6.3 Alert Logic

| Condition | Action |
| --- | --- |
| Lux below about 20,000 (cloudy, morning, evening) | Do not evaluate soiling; show "Low light" |
| Rolling-average soiling index above 15% for at least 5 consecutive minutes | **"Cleaning Required"**: buzzer + LED + dashboard + phone notification |
| Soiling index back below 8% | Clear the alert (hysteresis prevents flickering) |
| Test-panel current near zero while reference is normal | **"Fault"**: possible disconnection, shading or damage |
| Panel temperature above about 65 C | **"Overheating"** warning |

The 15% and 8% thresholds are starting values. Tune them from your own test data.

**Temperature note:** silicon panel output drops roughly 0.4% per degree C above 25 C. Because both panels sit side by side at nearly the same temperature, the ratio method cancels most of this effect. The temperature reading is mainly used for the overheating warning and for the dashboard.

### 6.4 Code Skeleton (Arduino / ESP32)

```cpp
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>

Adafruit_INA219 inaTest(0x40);
Adafruit_INA219 inaRef(0x41);
BH1750 lightMeter;
OneWire oneWire(4);
DallasTemperature tempSensor(&oneWire);
WiFiClient client;

const char* ssid = "YOUR_WIFI";
const char* pass = "YOUR_PASSWORD";
unsigned long channelId = 0;          // your ThingSpeak channel
const char* writeKey = "YOUR_KEY";

float k = 1.0;                         // calibration ratio, set when both panels are clean
const float LUX_MIN = 20000;
const float SOIL_ALERT = 15.0, SOIL_CLEAR = 8.0;
bool alertOn = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  inaTest.begin(); inaRef.begin();
  lightMeter.begin();
  tempSensor.begin();
  pinMode(25, OUTPUT); pinMode(26, OUTPUT);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  ThingSpeak.begin(client);
}

void loop() {
  float vT = inaTest.getBusVoltage_V();
  float iT = inaTest.getCurrent_mA();
  float vR = inaRef.getBusVoltage_V();
  float iR = inaRef.getCurrent_mA();
  float lux = lightMeter.readLightLevel();
  tempSensor.requestTemperatures();
  float tC = tempSensor.getTempCByIndex(0);

  float pT = vT * iT / 1000.0;         // watts
  float pR = vR * iR / 1000.0;

  float soiling = 0;
  bool valid = (lux > LUX_MIN && iR > 5);
  if (valid) soiling = (1.0 - (iT / iR) / k) * 100.0;

  if (valid && soiling > SOIL_ALERT) alertOn = true;     // add a 5-min average in the final version
  if (valid && soiling < SOIL_CLEAR) alertOn = false;
  digitalWrite(25, alertOn); digitalWrite(26, alertOn);

  ThingSpeak.setField(1, vT);
  ThingSpeak.setField(2, iT);
  ThingSpeak.setField(3, pT);
  ThingSpeak.setField(4, lux);
  ThingSpeak.setField(5, tC);
  ThingSpeak.setField(6, soiling);
  ThingSpeak.setField(7, alertOn ? 1 : 0);
  ThingSpeak.writeFields(channelId, writeKey);

  delay(20000);                         // ThingSpeak free tier needs at least 15 s between updates
}
```

This is a starting skeleton. Add sample averaging, the rolling window, OLED output, calibration storage (Preferences library) and Wi-Fi reconnect handling in the final build.

### 6.5 Software Modules

| Module | Responsibility | Runs |
| --- | --- | --- |
| Sensor drivers | Initialize and read both INA219s, the BH1750 and the DS18B20 | Every cycle |
| Signal processing | Average several samples, reject out-of-range values, calculate power = V x I | Every cycle |
| Soiling calculation | Check light level, then compute the soiling index against calibration constant k | Every cycle |
| Alert engine | Rolling average, thresholds, hysteresis, fault and overheating checks | Every cycle |
| Local output | Update the OLED, drive the buzzer and LED | Every cycle |
| Communication | Connect and reconnect Wi-Fi, upload fields to the cloud | Every 20 s |
| Calibration and storage | Measure k with both panels clean and save it to flash memory (Preferences library) | Once, and on demand |

Keep each module as its own function so you can test and explain them separately.

### 6.6 Cloud and Dashboard Setup (ThingSpeak Example)

1. Create a free ThingSpeak account and a new channel.
2. Enable seven fields: voltage, current, power, lux, temperature, soiling %, alert.
3. Copy the channel ID and the Write API key into the firmware.
4. Add charts and gauges to the channel view for each field.
5. For phone or email alerts, use ThingSpeak's React / ThingHTTP features, or Blynk's notification events if you choose Blynk. The exact steps depend on the platform's current interface, so follow its latest documentation.

### 6.7 Testing the Software Before Using Real Sensors

- Add a **test mode** to the firmware that feeds fixed values instead of sensor readings, for example I\_test / I\_ref = 0.80 with lux = 40,000. Confirm the soiling index shows 20% and the alert triggers, then confirm it clears when the ratio returns to 1.0.
- Feed a low-light value (lux = 5,000) and confirm no alert is raised.
- Disconnect Wi-Fi in the test and confirm the device reconnects and resumes uploads.
- Use Wokwi for the parts it simulates, and keep the final checks for the real hardware.

**Software is done when:** the test mode gives the expected index and alerts, readings reach the dashboard every 20 s, and a Wi-Fi drop recovers by itself.

## 7. Dashboard Design

| Widget | Data |
| --- | --- |
| Gauges | Voltage (V), Current (mA), Power (W) |
| Line graph | Power vs time, test panel vs reference panel |
| Line graph | Soiling index (%) vs time |
| Gauge / value | Lux and panel temperature |
| Status tile | OK / Cleaning Required / Fault / Overheating |
| Notification | Phone push or email on "Cleaning Required" |
| History | Daily energy (Wh) and cleaning events |

## 8. Workflow: From Idea to Submission

```
1. Buy parts -> 2. Test each sensor alone -> 3. Wire full circuit -> 4. Read all sensors
      -> 5. Calibrate (both panels clean) -> 6. Soiling + alert logic -> 7. Wi-Fi + dashboard
      -> 8. Outdoor test: clean vs dusty -> 9. Tune thresholds -> 10. Enclosure, report, PPT, demo
```

### Suggested Timeline (about 3 weeks)

| Week | Tasks |
| --- | --- |
| 1 | Order parts, simulate in Wokwi, test each sensor individually, wire the circuit |
| 2 | Write full firmware, calibrate, set up dashboard and alerts |
| 3 | Outdoor testing with clean vs dusty panel, tune thresholds, enclosure, report and PPT |

## 9. Testing and Validation Plan

| Test | Method | Expected result |
| --- | --- | --- |
| Sensor accuracy | Compare INA219 readings against a multimeter | Within a few percent |
| Calibration | Both panels clean, full sun, record k | k close to 1.0 (for example 0.95-1.05) |
| Soiling response | Sprinkle fine soil or flour evenly on the test panel | Soiling index rises; alert triggers above threshold |
| Cleaning recovery | Wipe the test panel | Index returns near 0%; alert clears |
| Cloud test | Cover both panels with a cloth or run on a cloudy day | Index not evaluated (low light), no false alert |
| Fault test | Disconnect the test panel | "Fault" status shown |
| Connectivity | Turn Wi-Fi off and on | Device reconnects and resumes uploads |
| Long run | Run for several days | Daily graph showing gradual soiling trend |

Log all results in a table or spreadsheet. Your clean-vs-dusty data is the main proof that the project works.

## 10. Risks and Limitations

| Risk | Probability | Impact | Mitigation |
| --- | --- | --- | --- |
| Panel mismatch makes the ratio unreliable | Medium | High | Use an identical pair and calibrate k when both are clean |
| False alerts on cloudy days or partial shade | High | Medium | Light threshold, rolling average, hysteresis |
| Sensor noise | Medium | Medium | Average multiple samples per reading |
| Wi-Fi drops | Medium | Low | Reconnect logic, store latest value, retry |
| Water or dust damages electronics | Medium | High | Enclosure, sealed connectors |
| Reference panel also gets dirty over time | High | Medium | Wipe it regularly and note it in the procedure |
| ThingSpeak free-tier rate limit | Medium | Low | Upload every 20 s or longer |
| Pump and relay cause power problems (optional) | Medium | Medium | Separate supply for pump, flyback protection |

**Known limitations:** soiling is estimated indirectly; results are for a small prototype and are not validated for large installations; accuracy depends on calibration; a damp or unevenly dusty panel can behave differently from the test-soil used in the lab.

## 11. Future Scope

- Automatic cleaning with a pump, wiper or brush motor
- Battery and charge-controller monitoring
- Machine-learning model for output prediction and anomaly detection
- LoRa or GSM telemetry for sites without Wi-Fi
- Monitoring multiple panels or strings with one gateway
- Weather API integration (dust storms, rain forecasts)
- Weather-aware cleaning schedule to save water

## 12. Deliverables Checklist

- Working hardware prototype in an enclosure
- Circuit diagram and wiring table
- Complete firmware with comments
- Dashboard with live link or screenshots
- Test data: clean vs dusty graphs and tables
- Project report (abstract, introduction, literature survey, block diagram, methodology, results, conclusion)
- PPT for presentation
- Demo video (also useful as a portfolio piece)
- Viva question list (Section 13)

## 13. Likely Viva Questions

1. **Why use INA219 instead of ACS712?** It measures voltage and current digitally over I2C with good resolution at low currents; ACS712 is noisy for small panel currents.
2. **Why two panels?** The reference panel gives a clean baseline under the same sunlight, so the difference is due to dust and not weather.
3. **Why not just use a dust sensor?** Optical dust sensors measure airborne dust, not the layer on the panel surface.
4. **How do you avoid false alerts on cloudy days?** Light threshold from the BH1750, rolling average and hysteresis.
5. **What is the calibration constant k?** The ratio of the two panels' currents when both are clean; it corrects for small differences between panels.
6. **How does temperature affect the panel?** Output voltage falls as temperature rises (about 0.4% per degree C for power); the side-by-side ratio method cancels most of it.
7. **What is the accuracy of your system?** Quote your measured results against a multimeter and your clean-vs-dusty test data.
8. **How can this scale?** Multiple nodes reporting to one cloud dashboard; LoRa for remote sites.

## 14. Freelance Pricing (Estimates)

These are market estimates for a beginner freelancer serving B.Tech students in India. They are not verified rates; adjust to your market.

| Package | Without hardware (code, diagram, dashboard, documentation) | With hardware (built and delivered) |
| --- | --: | --: |
| Basic (sensors, ESP32, dashboard) | Rs 3,000-5,000 | Rs 6,000-8,000 |
| **Recommended (+ soiling logic, alerts, report)** | Rs 5,000-7,000 | Rs 8,000-12,000 |
| Advanced (+ auto-cleaning pump) | Rs 7,000-10,000 | Rs 12,000-18,000 |

- With hardware, price at parts cost plus 2-3 times that for your time.
- Charge extra for the report, PPT, viva preparation and revisions.
- Take 50% advance and the balance on delivery.
- Include a walkthrough session so the student can explain and demo the project. A student who cannot explain their own project can fail the viva, and that hurts your reputation.

## 15. Integration: How Hardware and Software Work Together

The system runs as one repeating loop: the hardware measures, the firmware turns readings into a soiling index and an alert decision, the cloud stores and shows the result, and the user gets notified.

### 15.1 Layer-by-Layer View

| Layer | Hardware side | Software side | Passed to the next layer |
| --- | --- | --- | --- |
| Sensing | Panels, INA219 shunts, BH1750, DS18B20 | None (physical signals) | Current, voltage, light, temperature |
| Acquisition | I2C bus, 1-Wire pin | Wire, INA219, BH1750 and DallasTemperature libraries | Digital values: V, mA, lux, C |
| Processing | ESP32 CPU | Averaging, power = V x I, validity check, soiling index | Processed values and status |
| Decision | ESP32 CPU | Alert engine: thresholds, rolling average, hysteresis | Alert flag |
| Local output | OLED, buzzer, LED | Display and GPIO code | Visible and audible alert |
| Communication | ESP32 Wi-Fi radio, router | WiFi and ThingSpeak libraries | HTTP upload every 20 s |
| Cloud and user | Cloud server, phone or PC | Dashboard widgets, notifications | Graphs and alerts to the user |

### 15.2 Bring-Up Order: How to Combine Them

1. **Hardware alone:** the I2C scan finds 0x23, 0x3C, 0x40 and 0x41, and the DS18B20 responds.
2. **Raw readings:** print every sensor to the Serial Monitor and compare with a multimeter and a thermometer.
3. **Calibration:** with both panels clean in full sun, run calibration and store k.
4. **Processing:** confirm power, soiling index and the light-level check on Serial and the OLED.
5. **Alert logic:** force a test (partly cover the test panel, or use test mode) and confirm the buzzer, LED and OLED status.
6. **Wi-Fi and cloud:** connect, upload, and confirm all seven fields appear on the dashboard.
7. **Dashboard and notification:** add the widgets and confirm a phone or email alert arrives.
8. **Enclose and run outdoors:** leave it running for several days and compare against your clean-vs-dusty tests.

Test each step before adding the next, so any fault can be traced to one layer.

### 15.3 One Measurement Cycle

The times below are approximate and depend on your Wi-Fi and sample count.

| Step | Approx. time | Hardware does | Software does |
| --- | --- | --- | --- |
| 1. Sense | 0 s | Panels push current through the load resistors; INA219s, BH1750 and DS18B20 measure | Nothing |
| 2. Read | under 1 s | I2C and 1-Wire carry digital values to the ESP32 | Drivers read values and average several samples |
| 3. Compute | about 1 s | ESP32 CPU | Power, light-level check, soiling index against k, rolling average |
| 4. Decide | about 1 s | Nothing | Compare with the 15% and 8% thresholds; check fault and overheating |
| 5. Act locally | about 1 s | OLED updates; buzzer and LED switch | Display and GPIO code |
| 6. Send | 1-3 s | Wi-Fi radio sends to the router and internet | HTTP write of seven fields |
| 7. Show | a few seconds | Cloud stores the data; phone or PC displays it | Graphs update; notification if the alert flag is 1 |
| 8. Wait | rest of the 20 s | Idle | Delay until the next cycle |

### 15.4 Worked Example: Dust Building Up Over a Week

The numbers are illustrative, with k = 1.00 and good sunlight.

- **Day 1, clean:** I\_test / I\_ref = 1.00, soiling index 0%. Status OK.
- **Day 4, light dust:** ratio 0.92, soiling index 8%. Status OK, but the dashboard graph shows the rising trend.
- **Day 7, heavy dust:** ratio 0.80, soiling index 20% for more than 5 minutes. The alert fires: buzzer, LED, "Cleaning Required" on the dashboard, and a phone notification.
- **After cleaning:** ratio returns to about 1.00, the index drops below 8%, and the alert clears.
- **Cloudy afternoon:** lux falls below 20,000, so the same readings are ignored and no false alert is raised.

### 15.5 Common Integration Problems and Fixes

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| I2C scan finds no devices | Wrong SDA/SCL pins, no common GND, or no 3.3 V | Recheck wiring and shared ground |
| One INA219 missing or both read the same | Both are at address 0x40 | Bridge the A0 pad on the second one to VCC |
| Current reads zero or negative | Shunt wired backwards | Swap VIN+ and VIN- |
| DS18B20 reads -127 C | Missing pull-up or loose data wire | Add a 4.7 kOhm resistor between data and 3.3 V |
| Soiling index jumps around | Noise, partial shade or low light | Average more samples; keep the light threshold and rolling average |
| k is far from 1.0 | Panels mismatched, shaded or at different tilt | Fix the mounting, swap the panel if needed, and recalibrate |
| No data on the dashboard | Wrong channel ID or key, 5 GHz network, or uploads faster than 15 s | Check credentials, use a 2.4 GHz network, keep a 20 s interval |
| ESP32 resets when the pump starts | Pump shares the ESP32 supply | Use a separate pump supply, common ground, and a flyback diode |

### 15.6 Final Integration Checklist

- [ ] I2C scan shows 0x23, 0x3C, 0x40 and 0x41
- [ ] All readings match the multimeter and thermometer within a few percent
- [ ] k is stored and close to 1.0
- [ ] OLED shows live values and status
- [ ] Alert triggers on a test and clears afterwards
- [ ] Data reaches the dashboard every 20 s
- [ ] Phone or email notification is received
- [ ] System runs 24 hours unattended without a reset

## 16. Conclusion

The system combines low-cost sensors, an ESP32 and cloud telemetry to turn a solar panel into a self-monitoring unit. By comparing a dusty panel against a clean reference and filtering by sunlight, it gives a defensible, data-backed signal for when cleaning is worthwhile, at a prototype cost of about Rs 3,000-4,500.
