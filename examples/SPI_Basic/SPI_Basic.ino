/**
 * 7Semi LIS3DH - SPI Basic Example
 *
 * - Reads acceleration (raw + g) using SPI interface
 * - Reads temperature (optional)
 *
 * Wiring (SPI): Arduino 
 * - CS    -> any GPIO (example uses GPIO 10)
 * - SCL   -> SCK      (GPIO 13)
 * - SDO   -> MOSI     (GPIO 12)
 * - SDA   -> MISO     (GPIO 11)
 * - VCC   -> 3.3V / 5V (depends on module)
 * - GND   -> GND
 */

#include <7Semi_LIS3DH.h>

uint8_t spi_cs   = 10;
uint8_t spi_miso = 0xFF;
uint8_t spi_mosi = 0xFF;
uint8_t spi_sck  = 0xFF;
uint32_t spi_speed = 1000000;

// for ESP32
// uint8_t spi_cs   = 5;
// uint8_t spi_miso = 19;
// uint8_t spi_mosi = 23;
// uint8_t spi_sck  = 18;
// uint32_t spi_speed = 1000000;

LIS3DH_7Semi Adx;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("7Semi LIS3DH SPI Basic Example");

    if (!Adx.begin(SPI))
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
    else
    {
        Serial.println("RAW accel read failed!");
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
    else
    {
        Serial.println("G conversion failed!");
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
