/**
 * 7Semi LIS3DH - I2C Basic Example
 *
 * - Reads acceleration (raw + g) using I2C interface
 * - Reads temperature (optional)
 *
 * Wiring (I2C):
 * - SDA   -> SDA
 * - SCL   -> SCL
 * - VCC   -> 3.3V / 5V (depends on module)
 * - GND   -> GND
 *
 * Notes:
 * - Default I2C address is 0x18 (can be 0x19 depending on SA0 pin)
 */

#include <7Semi_LIS3DH.h>

uint8_t i2c_addr = 0x18;
uint32_t i2c_speed = 400000;

// for ESP32 custom pins
// uint8_t i2c_sda = 21;
// uint8_t i2c_scl = 22;

LIS3DH_7Semi Adx;

void setup()
{
    Serial.begin(115200);
    Serial.println("7Semi LIS3DH I2C Basic Example");

    if (!Adx.begin(Wire))
    {
        Serial.println("LIS3DH init failed!");
        while (1) delay(100);
    }

    uint8_t chipID = 0;
    if (Adx.readID(chipID))
    {
        Serial.print("Chip ID: 0x");
        Serial.println(chipID, HEX);
    }

    Adx.setScale(RANGE_2G);
    Adx.setDataRate(ODR_100HZ);
    Adx.enableTemperature(true);

    Serial.println("Sensor configured successfully!");
}

void loop()
{
    int16_t xRaw, yRaw, zRaw;
    float xG, yG, zG;

    if (Adx.readAccel(xRaw, yRaw, zRaw))
    {
        Serial.print("RAW -> X: ");
        Serial.print(xRaw);
        Serial.print(" Y: ");
        Serial.print(yRaw);
        Serial.print(" Z: ");
        Serial.println(zRaw);
    }

    if (Adx.getAccelerationX(xG) &&
        Adx.getAccelerationY(yG) &&
        Adx.getAccelerationZ(zG))
    {
        Serial.print("G   -> X: ");
        Serial.print(xG, 3);
        Serial.print("g Y: ");
        Serial.print(yG, 3);
        Serial.print("g Z: ");
        Serial.print(zG, 3);
        Serial.println("g");
    }
    

    float temp;
    if (Adx.getTemperature(temp))
    {
        Serial.print("Temp: ");
        Serial.print(temp, 2);
        Serial.println(" C");
    }

    Serial.println("---------------------");
    delay(1000);
}
