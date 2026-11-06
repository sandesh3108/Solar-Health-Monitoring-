// I2C bus scanner - run this FIRST during hardware bring-up.
// Expected devices: 0x23 (BH1750), 0x40 (INA219 test), 0x41 (INA219 ref),
// and 0x3C (OLED) once the v2 display is wired.
// Same pins as the main firmware (see documentation/03_hardware.md).
#include <Wire.h>

#define PIN_SDA 21
#define PIN_SCL 22

void setup() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("\nI2C scanner running - expect 0x23, 0x40, 0x41 (+0x3C)");
}

void loop() {
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  0x%02X  <- device found\n", addr);
      found++;
    }
  }
  Serial.printf("total: %d device(s)\n\n", found);
  delay(5000);
}
