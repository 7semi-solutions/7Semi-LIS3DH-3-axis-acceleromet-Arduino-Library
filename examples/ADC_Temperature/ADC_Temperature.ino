/**
 * 7Semi LIS3DH - ADC + Temperature Example
 *
 * - Enables ADC block
 * - Reads ADC1, ADC2, ADC3 outputs
 * - Reads temperature output (ADC3 based)
 *
 * Wiring (I2C):
 * - SDA   -> SDA
 * - SCL   -> SCL
 * - VCC   -> 3.3V / 5V
 * - GND   -> GND
 */

#include <7Semi_LIS3DH.h>

LIS3DH_7Semi Adx;

uint8_t i2c_addr = 0x18;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("7Semi Adx ADC + Temperature Example");

    if (!Adx.begin(Wire, i2c_addr))
    {
        Serial.println("LIS3DH init failed!");
        while (1) delay(100);
    }

    uint8_t chipID = 0;
    Adx.readID(chipID);
    Serial.print("Chip ID: 0x");
    Serial.println(chipID, HEX);

    /**
    * Enable ADC block.
    * The internal temperature sensor is connected to ADC3.
    * Temperature sensor and ADC3 channel cannot be used simultaneously.
    * If you want to use the internal temperature sensor, do not connect
    * any external signal to ADC3.
    */
    if (Adx.enableTemperature(true))
        Serial.println("Temperature enabled successfully");
    else
        Serial.println("Temperature enable failed");
}

void loop()
{
    int16_t adc1, adc2, adc3;
    float temp;

    if (Adx.readADC(adc1, adc2, adc3))
    {
        Serial.print("ADC1: ");
        Serial.print(adc1);
        Serial.print("  ADC2: ");
        Serial.print(adc2);
        Serial.print("  ADC3: ");
        Serial.println(adc3);
    }
    else
    {
        Serial.println("ADC read failed!");
    }

    if (Adx.getTemperature(temp))
    {
        Serial.print("Temperature: ");
        Serial.print(temp, 2);
        Serial.println(" C");
    }
    else
    {
        Serial.println("Temperature read failed!");
    }

    Serial.println("---------------------");
    delay(1000);
}
