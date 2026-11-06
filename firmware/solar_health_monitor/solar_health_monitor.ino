// ============================================================================
// solar_health_monitor - Smart Solar Panel Health & Dust Monitoring System
//
// ESP32 firmware. Minimal, layered:
//   * soiling_logic.h  -> tested core (validity gate, soiling %, rolling
//                         window, alert FSM) - shared with the PC unit tests
//   * config.h         -> pins, timing, Wi-Fi/cloud settings
//   * this file        -> thin glue: read sensors, log, upload
//
// Serial console (115200 baud):
//   c  calibrate k (both panels must be CLEAN, strong sun)
//   t  toggle test mode (fake readings: soiling = 20 %)
//   p  print one status line now
//   x  erase stored calibration
//   r  reset the 5-minute rolling window
//
// Wiring: see documentation/03_hardware.md
// v2 backlog (not implemented yet): OLED, buzzer/LED, relay/pump
// ============================================================================
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"
#include "soiling_logic.h"

using namespace solar;

// ---- hardware -------------------------------------------------------------
Adafruit_INA219 inaTest(ADDR_INA_TEST);
Adafruit_INA219 inaRef(ADDR_INA_REF);
BH1750 lightMeter;
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature tempSensor(&oneWire);
Preferences prefs;

// ---- state ----------------------------------------------------------------
Config cfg;                 // thresholds = defaults in soiling_logic.h
AlertEngine engine(cfg);
bool testMode = false;
unsigned long tRead = 0, tUpload = 0, tWifi = 0;

struct Raw {
  float vT = 0, iT = 0, vR = 0, iR = 0, lux = 0, tC = 0;
};
Raw raw;
Reading lastR;
Decision lastD = {ST_OK, false, 0.0f};

// ---- acquisition ----------------------------------------------------------
static void readAveraged(Adafruit_INA219& ina, float& v, float& i) {
  float vs = 0, is = 0;
  for (int n = 0; n < N_SAMPLES; n++) {
    vs += ina.getBusVoltage_V();
    is += ina.getCurrent_mA();
    delay(15);
  }
  v = vs / N_SAMPLES;
  i = fabs(is / N_SAMPLES);   // tolerate reversed shunt wiring
}

static void readSensors() {
  if (testMode) {             // deterministic demo: ratio 0.80 -> soiling 20 %
    raw.vT = 5.80f; raw.vR = 5.95f;
    raw.iT = 400.0f; raw.iR = 500.0f;
    raw.lux = 40000.0f; raw.tC = 41.0f;
    return;
  }
  readAveraged(inaTest, raw.vT, raw.iT);
  readAveraged(inaRef, raw.vR, raw.iR);
  raw.lux = lightMeter.readLightLevel();
  tempSensor.requestTemperatures();
  raw.tC = tempSensor.getTempCByIndex(0);
  if (raw.tC < -50.0f) raw.tC = 0.0f;   // DS18B20 wiring-error guard (-127)
}

static void evaluate() {
  Reading r;
  r.iTestMa = raw.iT;
  r.iRefMa = raw.iR;
  r.lux = raw.lux;
  r.tempC = raw.tC;
  lastD = engine.update(r, millis());
  lastR = r;
}

// ---- local output ---------------------------------------------------------
static const char* stateName(State s) {
  switch (s) {
    case ST_ALERT:    return "CLEANING!";
    case ST_FAULT:    return "FAULT!";
    case ST_OVERHEAT: return "OVERHEAT!";
    default:          return lastR.valid ? "OK" : "LOW LIGHT";
  }
}

static void printStatus() {
  Serial.printf("[%lu] VT=%.2f IT=%.0f VR=%.2f IR=%.0f P=%.2f lux=%.0f T=%.1f "
                "soil=%.1f avg=%.1f valid=%d state=%d wifi=%d\n",
                millis() / 1000, raw.vT, raw.iT, raw.vR, raw.iR,
                raw.vT * raw.iT / 1000.0f, raw.lux, raw.tC,
                lastR.soilPct, lastD.avgPct, (int)lastR.valid,
                (int)lastD.state, WiFi.status() == WL_CONNECTED);
}

// ---- calibration ----------------------------------------------------------
static void calibrate() {
  if (raw.lux <= cfg.luxMin || raw.iR <= cfg.iRefMinMa ||
      raw.iT <= cfg.iRefMinMa) {
    Serial.println("[cal] FAILED: need strong sun on BOTH clean panels");
    return;
  }
  float k = raw.iT / raw.iR;
  engine.setK(k);
  prefs.putFloat("k", k);
  prefs.putBool("calOK", true);
  Serial.printf("[cal] OK k=%.4f (stored in flash)\n", k);
}

// ---- cloud ----------------------------------------------------------------
static void wifiEnsure() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - tWifi < WIFI_RETRY_MS) return;
  tWifi = millis();
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

static void upload() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[cloud] skipped (no wifi)");
    return;
  }
  String url = String(CLOUD_URL);
  url += "?field1=" + String(raw.vT, 2);
  url += "&field2=" + String(raw.iT, 1);
  url += "&field3=" + String(raw.vT * raw.iT / 1000.0f, 2);
  url += "&field4=" + String(raw.lux, 0);
  url += "&field5=" + String(raw.tC, 1);
  url += "&field6=" + String(lastD.avgPct, 1);
  url += "&field7=" + String((int)lastD.state);

  HTTPClient http;
  http.setConnectTimeout(3000);
  http.setTimeout(3000);
  http.begin(url);
  int code = http.GET();
  http.end();
  if (code == 200) Serial.println("[cloud] ok");
  else Serial.printf("[cloud] error %d\n", code);
}

// ---- serial console -------------------------------------------------------
// NB: not named cli() - that collides with Arduino.h's cli() macro!
static void pollSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'c': case 'C':
      calibrate();
      break;
    case 't': case 'T':
      testMode = !testMode;
      engine.reset();
      Serial.printf("[test] mode %s\n", testMode ? "ON (soil=20%)" : "OFF");
      break;
    case 'p': case 'P':
      printStatus();
      break;
    case 'x': case 'X':
      prefs.remove("k");
      prefs.remove("calOK");
      engine.setK(1.0f);
      Serial.println("[cal] erased");
      break;
    case 'r': case 'R':
      engine.reset();
      Serial.println("[window] reset");
      break;
  }
}

// ---- setup / loop ---------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println("\n== Solar Health Monitor v1.0 ==");

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!inaTest.begin()) Serial.println("[hw] INA219 test (0x40) NOT found");
  if (!inaRef.begin()) Serial.println("[hw] INA219 ref  (0x41) NOT found");
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  tempSensor.begin();
  tempSensor.setResolution(12);

  prefs.begin("calib", false);
  engine.setK(prefs.getFloat("k", 1.0f));
  Serial.printf("[cal] k=%.3f stored=%d\n", engine.k(), (int)prefs.getBool("calOK", false));

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) delay(250);
  Serial.println(WiFi.status() == WL_CONNECTED ? "[wifi] connected"
                                               : "[wifi] FAILED - running offline");

  tRead = tUpload = millis();
}

void loop() {
  pollSerial();

  if (millis() - tRead >= READ_MS) {
    tRead = millis();
    readSensors();
    evaluate();
    printStatus();
  }

  if (millis() - tUpload >= UPLOAD_MS) {
    tUpload = millis();
    wifiEnsure();
    upload();
  }
}
