/**
 * 7Semi LIS3DH - INT1 Interrupt Example
 *
 * - Enables INT1 threshold interrupt
 * - Uses attachInterrupt() to detect motion
 * - Reads interrupt status register
 *
 * Wiring:
 * - SDA   -> SDA
 * - SCL   -> SCL
 * - VCC   -> 3.3V / 5V
 * - GND   -> GND
 *
 * Optional wiring:
 * - INT1 -> MCU interrupt pin (example uses D2)
 */
#include <7Semi_LIS3DH.h>

LIS3DH_7Semi adx;

uint8_t i2c_addr = 0x18;
uint8_t int1_pin = 2;

volatile bool intFlag = false;

void int1ISR() {
  intFlag = true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("7Semi LIS3DH INT1 Interrupt Example");

  if (!adx.begin(Wire, i2c_addr)) {
    Serial.println("LIS3DH init failed!");
    while (1) delay(100);
  }

  uint8_t chipID = 0;
  adx.readID(chipID);
  Serial.print("Chip ID: 0x");
  Serial.println(chipID, HEX);

  pinMode(int1_pin, INPUT);

  /**
     * Enable INT1 interrupt
     *
     * - active_high = true
     * - threshold = 100 mg
     * - X/Y/Z LOW threshold interrupt on Interrupt pin INT1
     * - X/Y/Z HIGH threshold interrupt on Interrupt pin INT2
     */
  if (adx.enableINT1(true, 100))
    Serial.println("INT1 enabled successfully");
  else
    Serial.println("INT1 enable failed");

  attachInterrupt(digitalPinToInterrupt(int1_pin), int1ISR, RISING);

  Serial.println("Waiting for INT1 trigger...");
}

void loop() {
  if (intFlag) {
    intFlag = false;
    int16_t xRaw, yRaw, zRaw;
    // float xG, yG, zG;

    if (adx.readAccel(xRaw, yRaw, zRaw)) {
      Serial.print("RAW -> X: ");
      Serial.print(xRaw);
      Serial.print(" Y: ");
      Serial.print(yRaw);
      Serial.print(" Z: ");
      Serial.println(zRaw);
    }
    uint8_t status = 0;
    if (adx.readInt1Status(status)) {
      Serial.print("INT1 Triggered! Status: 0x");
      Serial.println(status, HEX);

      if (status & (1 << 6))
        Serial.println("Interrupt Active");

      if (status & (1 << 5))
        Serial.println("Z High");

      if (status & (1 << 4))
        Serial.println("Z Low");

      if (status & (1 << 3))
        Serial.println("Y High");

      if (status & (1 << 2))
        Serial.println("Y Low");

      if (status & (1 << 1))
        Serial.println("X High");

      if (status & (1 << 0))
        Serial.println("X Low");
    }

    adx.clearInt1();  // Reading INT1_SRC clears the interrupt
    Serial.println("---------------------");
  }

  delay(500);
}
