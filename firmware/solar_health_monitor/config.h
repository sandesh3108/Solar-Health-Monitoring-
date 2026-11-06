#pragma once
// ============================================================================
// config.h - firmware-level configuration (pins, timing, credentials)
// Logic thresholds live in soiling_logic.h (solar::Config) so that the PC
// unit tests and the firmware share ONE definition of every threshold.
// ============================================================================

// ---- pins -----------------------------------------------------------------
#define PIN_SDA 21          // I2C data  (INA219 x2, BH1750)
#define PIN_SCL 22          // I2C clock
#define PIN_ONEWIRE 4       // DS18B20 data (needs 4.7 kOhm pull-up to 3.3 V)

// ---- timing (ms) ----------------------------------------------------------
#define SERIAL_BAUD 115200
#define READ_MS 5000UL      // sensor read + decision cadence
#define UPLOAD_MS 20000UL   // cloud upload cadence (ThingSpeak free tier >= 15 s)
#define WIFI_RETRY_MS 60000UL

// ---- acquisition ----------------------------------------------------------
#define N_SAMPLES 8         // samples averaged per INA219 reading

// ---- I2C addresses (verify with tools/i2c_scanner) ------------------------
#define ADDR_INA_TEST 0x40  // test panel INA219   (A0 -> GND)
#define ADDR_INA_REF 0x41   // reference INA219    (A0 -> VCC)
#define ADDR_BH1750 0x23    // light sensor        (ADDR low)

// ---- credentials / endpoints  [EDIT BEFORE FLASHING] ----------------------
static const char* WIFI_SSID = "YOUR_WIFI";
static const char* WIFI_PASS = "YOUR_PASSWORD";
// Mock cloud = mock_cloud/server.py on your PC (use your PC's LAN IP).
// Later this single line can point at ThingSpeak: https://api.thingspeak.com/update
static const char* CLOUD_URL = "http://192.168.1.5:5000/update";
