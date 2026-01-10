/**
 * 7Semi LIS3DH Library
 *
 * - Driver for LIS3DH 3-axis accelerometer
 * - Supports I2C and SPI communication
 * - Provides acceleration, interrupts, ADC and temperature features
 *
 * Notes:
 * - Default WHO_AM_I value for LIS3DH is 0x33
 * - Supports INT1 and INT2 threshold based interrupts
 * - ADC and Temperature share TEMP_CFG_REG enable bits
 */
#include "7Semi_LIS3DH.h"

TwoWire *LIS3DH_7Semi::i2c = nullptr;
SPIClass *LIS3DH_7Semi::spi = nullptr;
uint8_t LIS3DH_7Semi::cs_pin = 0;
uint8_t LIS3DH_7Semi::sensor_address = 0;

/**
 * Constructor
 *
 * - Initializes default configuration
 * - Default bus mode is I2C
 * - Default accelerometer range is ±2g
 * - Default speed is 400kHz
 */
LIS3DH_7Semi::LIS3DH_7Semi()
{
    acc_scale = RANGE_2G;
    bus = I2C_bus;
    cs_pin = 0;
    speed = 400000;
}

/**
 * Initialize LIS3DH over I2C
 *
 * Inputs:
 * - wire: Wire instance (example: Wire)
 * - i2c_address: device address (default 0x18)
 * - i2c_sda: optional SDA pin 
 * - i2c_scl: optional SCL pin 
 * - i2c_speed: I2C clock speed (default 400kHz)
 *
 * Return:
 * - true  : sensor detected and configured successfully
 * - false : sensor not detected or communication failed
 *
 * Notes:
 * - This checks WHO_AM_I register (must be 0x33)
 * - Applies default output settings: 100Hz + HR mode
 */
bool LIS3DH_7Semi::begin(TwoWire &wire, uint8_t i2c_address,
                         uint8_t i2c_sda, uint8_t i2c_scl,
                         uint32_t i2c_speed)
{
    bus = I2C_bus;
    i2c = &wire;
    sensor_address = i2c_address;
    speed = i2c_speed;

#if defined(ESP32) || defined(ESP8266)
    if (i2c_sda != 0xFF && i2c_scl != 0xFF)
        i2c->begin(i2c_sda, i2c_scl);
    else
        i2c->begin();
#else
    (void)i2c_sda;
    (void)i2c_scl;
    i2c->begin();
#endif

    i2c->setClock(speed);

    /**
     * Check device ID
     *
     * - Reads WHO_AM_I register
     * - Must return 0x33 for LIS3DH
     */

    uint8_t device_id = 0;
    if (!readID(device_id))
        return false;
    if (device_id != 0x33)
        return false;

    /**
     * Prevent unused parameter warnings
     */


    /**
     * Default configuration
     *
     * - CTRL_REG0: enable pull-up / internal filter options
     * - TEMP_CFG_REG: enable temperature/ADC block
     * - CTRL_REG1: 100Hz + XYZ enable
     * - CTRL_REG4: high resolution mode + range
     */
    writeReg(CTRL_REG0, 0x10);
    writeReg(TEMP_CFG_REG, 0x80);
    writeReg(CTRL_REG1, 0x57);
    writeReg(CTRL_REG3, 0x00);
    writeReg(CTRL_REG4, 0x88);

    return true;
}


/**
 * Initialize LIS3DH over SPI
 *
 * Inputs:
 * - spiPort: SPI instance (example: SPI)
 * - spi_cs: chip select pin
 * - spi_sck: optional SPI SCK pin (ESP32 only)
 * - spi_miso: optional SPI MISO pin (ESP32 only)
 * - spi_mosi: optional SPI MOSI pin (ESP32 only)
 * - spi_speed: SPI clock frequency (default 1MHz)
 *
 * Return:
 * - true  : sensor detected and configured successfully
 * - false : sensor not detected or communication failed
 *
 * Notes:
 * - Uses SPI Mode 3 as per LIS3DH datasheet
 * - Checks WHO_AM_I (0x33)
 */
bool LIS3DH_7Semi::begin(SPIClass &spiPort, uint8_t spi_cs,
                         uint8_t spi_sck, uint8_t spi_miso,
                         uint8_t spi_mosi, uint32_t spi_speed)
{
    bus = SPI_bus;
    spi = &spiPort;
    cs_pin = spi_cs;

    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);

#if defined(ESP32)
    if (spi_sck != 0xFF && spi_miso != 0xFF && spi_mosi != 0xFF)
        spi->begin(spi_sck, spi_miso, spi_mosi, cs_pin);
    else
        spi->begin();
#else
    (void)spi_sck;
    (void)spi_miso;
    (void)spi_mosi;
    spi->begin();
#endif

    /**
     * SPI transaction settings
     *
     * - MSB first
     * - SPI Mode 3
     */
    spi->beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE3));

    /**
     * Prevent unused parameter warnings
     */


    /**
     * Check device ID
     */
    uint8_t device_id = 0;
    if (!readReg(WHO_AM_I, device_id))
        return false;
    if (device_id != 0x33)
        return false;

    /**
     * Default configuration
     */
    writeReg(CTRL_REG0, 0x10);
    writeReg(TEMP_CFG_REG, 0x80);
    writeReg(CTRL_REG1, 0x57);
    writeReg(CTRL_REG3, 0x00);
    writeReg(CTRL_REG4, 0x80);

    spi->endTransaction();

    return true;
}

/**
 * Read single register over I2C
 *
 * Inputs:
 * - reg: register address to read
 * - value: reference that stores read value
 *
 * Return:
 * - true  : read succeeded
 * - false : bus communication failed
 */
bool LIS3DH_7Semi::i2c_read(uint8_t reg, uint8_t &value)
{
    i2c->beginTransmission(sensor_address);
    i2c->write(reg);

    if (i2c->endTransmission(false) != 0)
        return false;

    i2c->requestFrom(sensor_address, (uint8_t)1);

    if (i2c->available())
        value = i2c->read();

    return true;
}

/**
 * Write single register over I2C
 *
 * Inputs:
 * - reg: register address to write
 * - value: value to write
 *
 * Return:
 * - true  : write succeeded
 * - false : bus communication failed
 */
bool LIS3DH_7Semi::i2c_write(uint8_t reg, uint8_t value)
{
    i2c->beginTransmission(sensor_address);
    i2c->write(reg);
    i2c->write(value);
    return (i2c->endTransmission() == 0);
}


 /**
 * Read single register over SPI
 *
 * Inputs:
 * - reg: register address to read
 * - value: reference that stores read value
 *
 * Return:
 * - true : always returns true unless SPI object invalid
 *
 */
bool LIS3DH_7Semi::spi_read(uint8_t reg, uint8_t &value)
{
    digitalWrite(cs_pin, LOW);
    spi->transfer(reg | 0x80);
    value = spi->transfer(0x00);
    digitalWrite(cs_pin, HIGH);
    return true;
}


/**
 * Write single register over SPI
 *
 * Inputs:
 * - reg: register address to write
 * - value: value to write
 *
 * Return:
 * - true : always returns true unless SPI object invalid
 */
bool LIS3DH_7Semi::spi_write(uint8_t reg, uint8_t value)
{
    digitalWrite(cs_pin, LOW);
    spi->transfer(reg & 0x7F);
    spi->transfer(value);
    digitalWrite(cs_pin, HIGH);
    return true;
}

/**
 * Read register based on bus mode
 *
 * Inputs:
 * - reg: register address
 * - val: reference to store read value
 *
 * Return:
 * - true  : success
 * - false : failed I2C transaction or invalid bus
 */
bool LIS3DH_7Semi::readReg(uint8_t reg, uint8_t &val)
{
    return (bus == I2C_bus) ? i2c_read(reg, val) : spi_read(reg, val);
}

/**
 * Write register based on bus mode
 *
 * Inputs:
 * - reg: register address
 * - val: value to write
 *
 * Return:
 * - true  : success
 * - false : failed I2C transaction or invalid bus
 */
bool LIS3DH_7Semi::writeReg(uint8_t reg, uint8_t val)
{
    return (bus == I2C_bus) ? i2c_write(reg, val) : spi_write(reg, val);
}

/* ============================================================
 * Device Information
 * ============================================================ */

/**
 * Read device ID (WHO_AM_I)
 *
 * Inputs:
 * - id: reference that stores device ID
 *
 * Return:
 * - true  : read succeeded
 * - false : read failed (bus issue / sensor not responding)
 *
 * Notes:
 * - LIS3DH WHO_AM_I should return 0x33
 */
bool LIS3DH_7Semi::readID(uint8_t &id)
{
    return readReg(WHO_AM_I, id);
}

/**
 * Set accelerometer measurement range
 *
 * Inputs:
 * - scale: range selection constant
 *   - RANGE_2G, RANGE_4G, RANGE_8G, RANGE_16G
 *
 * Return:
 * - true  : register updated successfully
 * - false : invalid scale or bus write failed
 *
 * Notes:
 * - Updates CTRL_REG4 FS bits
 * - Keeps existing CTRL_REG4 configuration intact
 */
bool LIS3DH_7Semi::setScale(uint8_t scale)
{
    if (scale < 0x04)
    {
        uint8_t val;
        if (!readReg(CTRL_REG4, val))
            return false;

        /**
         * Clear existing FS bits and set new scale
         *
         * - FS bits are [5:4]
         */
        val &= ~(0x03 << 4);
        val |= (scale & 0x03) << 4;

        if (writeReg(CTRL_REG4, val))
        {
            acc_scale = scale;
            return true;
        }
    }
    return false;
}


/**
 * Set accelerometer output data rate
 *
 * Inputs:
 * - dataRate: ODR value (0x00 - 0x0F)
 *
 * Return:
 * - true  : CTRL_REG1 updated successfully
 * - false : invalid rate or bus error
 *
 * Notes:
 * - ODR bits are [7:4] inside CTRL_REG1
 * - Keeps axis enable bits intact
 */
bool LIS3DH_7Semi::setDataRate(uint8_t dataRate)
{
    if (dataRate < 0x0F)
    {
        uint8_t val;
        if (!readReg(CTRL_REG1, val))
            return false;

        /**
         * Clear existing ODR bits and set new ones
         *
         * - ODR bits = [7:4]
         */
        val &= ~(0x0F << 4);
        val |= (dataRate & 0x0F) << 4;

        if (writeReg(CTRL_REG1, val))
        {
            data_rate = dataRate;
            return true;
        }
    }
    return false;
}

/**
 * Set accelerometer Output Data Rate (ODR)
 *
 * Inputs:
 * - dataRate : Output data rate selection (0x00 – 0x0F)
 *
 * Return:
 * - true  : CTRL_REG1 updated successfully
 * - false : Invalid data rate or communication error
 *
 * Notes:
 * - ODR bits are located at CTRL_REG1[7:4]
 * - This function DOES NOT enable High-Resolution mode
 * - High-Resolution is controlled by CTRL_REG4 bit 3 (HR)
 * - Axis enable bits (X, Y, Z) are preserved
 */

bool LIS3DH_7Semi::setHighResolution(bool enable)
{
    uint8_t val;
    if (!readReg(CTRL_REG4, val))
        return false;

    if (enable)
        val |= (1 << 3);   // HR bit
    else
        val &= ~(1 << 3);

    return writeReg(CTRL_REG4, val);
}

/**
 * Read accelerometer data with mode alignment
 *
 * Inputs:
 * - x: reference to store aligned X axis value
 * - y: reference to store aligned Y axis value
 * - z: reference to store aligned Z axis value
 *
 * Return:
 * - true  : successfully read and aligned XYZ values
 * - false : bus communication failed
 *
 * Notes:
 * - Reads OUT_X_L to OUT_Z_H (6 bytes)
 * - Aligns raw data depending on operating mode:
 *   - Low Power: 8-bit
 *   - High Resolution: 12-bit
 *   - Normal: 10-bit
 */
bool LIS3DH_7Semi::readAccel(int16_t &x, int16_t &y, int16_t &z)
{
    uint8_t data[6];
    uint8_t addr = OUT_X_L;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (!readReg(addr + i, data[i]))
            return false;
    }

    int16_t rawX = (int16_t)((data[1] << 8) | data[0]);
    int16_t rawY = (int16_t)((data[3] << 8) | data[2]);
    int16_t rawZ = (int16_t)((data[5] << 8) | data[4]);

    /**
     * Read mode bits from CTRL_REG1 and CTRL_REG4
     */
    uint8_t val = 0, val1 = 0;
    readReg(CTRL_REG1, val);
    readReg(CTRL_REG4, val1);

    bool lp = val & (1 << 3);
    bool hr = val1 & (1 << 3);

    /**
     * Shift data based on selected mode resolution
     */
    if (lp)
    {
        rawX >>= 8;
        rawY >>= 8;
        rawZ >>= 8;
    }
    else if (hr)
    {
        rawX >>= 4;
        rawY >>= 4;
        rawZ >>= 4;
    }
    else
    {
        rawX >>= 6;
        rawY >>= 6;
        rawZ >>= 6;
    }

    x = rawX;
    y = rawY;
    z = rawZ;

    return true;
}

/**
 * Read accelerometer raw register values without alignment
 *
 * Inputs:
 * - x: reference to store raw X axis value
 * - y: reference to store raw Y axis value
 * - z: reference to store raw Z axis value
 *
 * Return:
 * - true  : raw values read successfully
 * - false : bus communication failed
 *
 * Notes:
 * - Does NOT apply shifting or mode alignment
 * - Useful for debugging register values
 */
bool LIS3DH_7Semi::readAccelRaw(int16_t &x, int16_t &y, int16_t &z)
{
    uint8_t buf[6];

    for (int i = 0; i < 6; i++)
        if (!readReg(OUT_X_L + i, buf[i]))
            return false;

    int16_t rx = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t ry = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t rz = (int16_t)((buf[5] << 8) | buf[4]);

    x = rx;
    y = ry;
    z = rz;

    return true;
}

/**
 * Read acceleration in g for X-axis
 *
 * Inputs:
 * - x: reference to store X-axis acceleration in g
 *
 * Return:
 * - true  : acceleration read successfully
 * - false : failed to read accel registers
 *
 * Notes:
 * - Uses aligned accel read + rawToG conversion
 */
bool LIS3DH_7Semi::getAccelerationX(float &x)
{
    int16_t rx, ry, rz;
    if (!readAccel(rx, ry, rz))
        return false;

    x = rawToG(rx);
    return true;
}

/**
 * Read acceleration in g for Y-axis
 *
 * Inputs:
 * - y: reference to store Y-axis acceleration in g
 *
 * Return:
 * - true  : acceleration read successfully
 * - false : failed to read accel registers
 */
bool LIS3DH_7Semi::getAccelerationY(float &y)
{
    int16_t rx, ry, rz;
    if (!readAccel(rx, ry, rz))
        return false;

    y = rawToG(ry);
    return true;
}

/**
 * Read acceleration in g for Z-axis
 *
 * Inputs:
 * - z: reference to store Z-axis acceleration in g
 *
 * Return:
 * - true  : acceleration read successfully
 * - false : failed to read accel registers
 */
bool LIS3DH_7Semi::getAccelerationZ(float &z)
{
    int16_t rx, ry, rz;
    if (!readAccel(rx, ry, rz))
        return false;

    z = rawToG(rz);
    return true;
}


/**
 * Convert raw axis value into g units
 *
 * Inputs:
 * - raw: signed raw accelerometer value
 *
 * Return:
 * - Converted acceleration in g
 *
 * Notes:
 * - Conversion depends on:
 *   - Current scale selection (±2g / ±4g / ±8g / ±16g)
 *   - Current operating mode (LP / Normal / HR)
 */
float LIS3DH_7Semi::rawToG(int16_t raw)
{
    float mg_per_lsb = 1.0f;

    uint8_t val, val1;
    readReg(CTRL_REG1, val);
    readReg(CTRL_REG4, val1);

    bool lowPower = val & (1 << 3);
    bool highRes  = val1 & (1 << 3);

    if (lowPower)
    {
        switch (acc_scale)
        {
            case RANGE_2G:  mg_per_lsb = 16;  break;
            case RANGE_4G:  mg_per_lsb = 32;  break;
            case RANGE_8G:  mg_per_lsb = 64;  break;
            case RANGE_16G: mg_per_lsb = 192; break;
        }
    }
    else if (highRes)
    {
        switch (acc_scale)
        {
            case RANGE_2G:  mg_per_lsb = 1;  break;
            case RANGE_4G:  mg_per_lsb = 2;  break;
            case RANGE_8G:  mg_per_lsb = 4;  break;
            case RANGE_16G: mg_per_lsb = 12; break;
        }
    }
    else
    {
        switch (acc_scale)
        {
            case RANGE_2G:  mg_per_lsb = 4;  break;
            case RANGE_4G:  mg_per_lsb = 8;  break;
            case RANGE_8G:  mg_per_lsb = 16; break;
            case RANGE_16G: mg_per_lsb = 48; break;
        }
    }

    /**
     * Convert mg to g
     *
     * - Multiply by 0.001 to convert mg → g
     */
    return raw * mg_per_lsb * 0.001f;
}

/**
 * Read ADC channel value
 *
 * Inputs:
 * - channel: ADC channel number (1 to 3)
 * - adcValue: reference stores ADC output (16-bit)
 *
 * Return:
 * - true  : ADC value read successfully
 * - false : invalid channel or bus read failed
 *
 * Notes:
 * - ADC output registers:
 *   - ADC1: OUT_ADC1_L/H
 *   - ADC2: OUT_ADC2_L/H
 *   - ADC3: OUT_ADC3_L/H
 */
bool LIS3DH_7Semi::readADC_CH(uint8_t channel, int16_t &adcValue)
{
    if (channel < 1 || channel > 3)
        return false;

    uint8_t channel_reg = OUT_ADC1_L + ((channel - 1) * 2);

    uint8_t lowByte, highByte;

    if (!readReg(channel_reg, lowByte))
        return false;
    if (!readReg(channel_reg + 1, highByte))
        return false;

    adcValue = (int16_t)((highByte << 8) | lowByte);

    return true;
}

/**
 * Read all ADC channels together
 *
 * Inputs:
 * - adcX: reference stores ADC1 value
 * - adcY: reference stores ADC2 value
 * - adcZ: reference stores ADC3 value
 *
 * Return:
 * - true  : all ADC values read successfully
 * - false : bus read failed
 *
 * Notes:
 * - Reads 6 bytes from OUT_ADC1_L to OUT_ADC3_H
 */
bool LIS3DH_7Semi::readADC(int16_t &adcX, int16_t &adcY, int16_t &adcZ)
{
    uint8_t data[6];

    for (uint8_t i = 0; i < 6; i++)
    {
        if (!readReg(OUT_ADC1_L + i, data[i]))
            return false;
    }

    adcX = (int16_t)((data[1] << 8) | data[0]);
    adcY = (int16_t)((data[3] << 8) | data[2]);
    adcZ = (int16_t)((data[5] << 8) | data[4]);

    return true;
}

/**
 * Enable or disable ADC block
 *
 * Inputs:
 * - enable:
 *   - true  : enables ADC / temperature block
 *   - false : disables ADC / temperature block
 *
 * Return:
 * - true  : register updated successfully
 * - false : bus read/write failed
 *
 * Notes:
 * - TEMP_CFG_REG bit7 enables ADC + temperature block
 */
bool LIS3DH_7Semi::enableADC(bool enable)
{
    uint8_t temp_cfg;

    if (!readReg(TEMP_CFG_REG, temp_cfg))
        return false;

    if (enable)
        temp_cfg |= 0x80;
    else
        temp_cfg &= ~0x80;

    return writeReg(TEMP_CFG_REG, temp_cfg);
}

/**
 * Enable or disable temperature sensor output
 *
 * Inputs:
 * - enable:
 *   - true  : enables temperature output (ADC3)
 *   - false : disables temperature output
 *
 * Return:
 * - true  : register updated successfully
 * - false : bus read/write failed
 *
 * Notes:
 * - TEMP_CFG_REG:
 *   - bit7 enables ADC + temperature block
 *   - bit6 enables temperature output
 */
bool LIS3DH_7Semi::enableTemperature(bool enable)
{
    uint8_t val;

    if (!readReg(TEMP_CFG_REG, val))
        return false;

    if (enable)
        val |= 0xC0;
    else
        val &= ~0x40;

    return writeReg(TEMP_CFG_REG, val);
}

/**
 * Read temperature value from ADC3
 *
 * Inputs:
 * - temperature: reference stores temperature output in Celsius
 *
 * Return:
 * - true  : temperature read successfully
 * - false : bus read failed or temperature enable failed
 *
 * Notes:
 * - Temperature output is available on OUT_ADC3_L/H
 * - Conversion depends on calibration
 * - This implementation returns raw_temp / 100 as approximation
 * - Automatically enables and disables temperature for reading
 */
bool LIS3DH_7Semi::getTemperature(float &temperature)
{
    // if (!enableTemperature(true))
    //     return false;

    uint8_t low, high;

    if (!readReg(OUT_ADC3_L, low))
        return false;
    if (!readReg(OUT_ADC3_H, high))
        return false;

    int16_t raw_temp = (int16_t)((high << 8) | low);

    /**
     * Approximate conversion into Celsius
     *
     * - User should recalibrate based on sensor output behavior
     */
    temperature = (float)raw_temp / 100.0f;

    /**
     * Disable after reading to reduce noise and power usage
     */
    // enableTemperature(false);

    return true;
}

/**
 * Enable and configure INT1 threshold interrupt
 *
 * Inputs:
 * - active_high: interrupt polarity selection
 *   - true  : active HIGH output
 *   - false : active LOW output
 * - threshold_mg: threshold value in milli-g (mg)
 *
 * Return:
 * - true  : interrupt successfully configured
 * - false : register write/read failed
 *
 * Notes:
 * - Enables XYZ axes
 * - Routes IA1 interrupt to INT1 pin using CTRL_REG3
 * - Sets pulse mode (non-latched)
 * - Uses INT1_CFG for XYZ high + low OR logic
 * - Threshold register depends on selected accelerometer range
 */
bool LIS3DH_7Semi::enableINT1(bool active_high, uint16_t threshold_mg)
{
    uint8_t val;

    /**
     * Enable X/Y/Z axes while keeping ODR unchanged
     */
    if (!readReg(CTRL_REG1, val))
        return false;
    val |= 0x07;
    if (!writeReg(CTRL_REG1, val))
        return false;

    /**
     * Disable high-pass filter
     */
    if (!writeReg(CTRL_REG2, 0x00))
        return false;

    /**
     * Route IA1 interrupt to INT1 pin
     */
    constexpr uint8_t INT1_IA1 = 0x40;
    if (!writeReg(CTRL_REG3, INT1_IA1))
        return false;

    /**
     * Use pulse mode (interrupt not latched)
     */
    if (!writeReg(CTRL_REG5, 0x00))
        return false;

    /**
     * Set interrupt polarity using CTRL_REG6
     *
     * - bit1 controls INT polarity
     */
    if (!readReg(CTRL_REG6, val))
        return false;

    if (active_high)
        val &= ~(1 << 1);
    else
        val |= (1 << 1);

    if (!writeReg(CTRL_REG6, val))
        return false;

    /**
     * Convert threshold from mg into register format
     *
     * - Threshold LSB depends on full-scale range
     */
    uint8_t ths_lsb;

    switch (acc_scale)
    {
        case RANGE_2G:  ths_lsb = threshold_mg / 16;  break;
        case RANGE_4G:  ths_lsb = threshold_mg / 32;  break;
        case RANGE_8G:  ths_lsb = threshold_mg / 62;  break;
        case RANGE_16G: ths_lsb = threshold_mg / 186; break;
        default:        ths_lsb = threshold_mg / 16;  break;
    }
    if (ths_lsb <= 0)
       ths_lsb = 1;

    /**
     * Clamp threshold register to valid range
     */
    if (ths_lsb > 0x7F)
        ths_lsb = 0x7F;

    if (!writeReg(INT1_THS, ths_lsb))
        return false;

    readReg(INT1_THS, ths_lsb);
    Serial.print("threshold_mg: ");
    Serial.println(ths_lsb);
    /**
     * Duration = 0 triggers interrupt immediately
     */
    if (!writeReg(INT1_DURATION, 0x00))
        return false;

    /**
     * Clear any pending interrupt by reading INT1_SRC
     */
    readReg(INT1_SRC, val);

    /**
     * Enable interrupt on X/Y/Z low threshold
     *
     * - OR logic enabled (no 6D mode)
     */
    constexpr uint8_t INT1_ZYX = 0x15;
    if (!writeReg(INT1_CFG, INT1_ZYX))
        return false;

    return true;
}

/**
 * Enable and configure INT2 interrupt
 *
 * Inputs:
 * - activeHigh: polarity selection
 *   - true  : active HIGH output
 *   - false : active LOW output
 * - threshold: threshold value (mg)
 *
 * Return:
 * - true  : interrupt successfully configured
 * - false : register read/write failed
 *
 * Notes:
 * - Routes interrupt to INT2 pin via CTRL_REG6
 * - Uses INT2_CFG for event configuration
 * - Threshold scaling depends on accelerometer range
 */
bool LIS3DH_7Semi::enableINT2(bool activeHigh, uint8_t threshold)
{
    uint8_t val;

    /**
     * Enable XYZ axes while keeping ODR unchanged
     */
    if (!readReg(CTRL_REG1, val))
        return false;
    val |= 0x07;
    if (!writeReg(CTRL_REG1, val))
        return false;

    /**
     * Route interrupt to INT2 pin using CTRL_REG6
     *
     * - bit4 enables INT2 routing
     * - bit2 sets polarity
     */
    if (!readReg(CTRL_REG6, val))
        return false;

    val |= (1 << 4);

    if (!activeHigh)
        val |= (1 << 2);
    else
        val &= ~(1 << 2);

    if (!writeReg(CTRL_REG6, val))
        return false;

    /**
     * Configure INT2 trigger conditions
     */
    if (!writeReg(INT2_CFG, 0x2A))
        return false;

    /**
     * Convert threshold into register units
     */
    uint8_t ths;
    switch (acc_scale)
    {
        case RANGE_2G:  ths = threshold / 16;  break;
        case RANGE_4G:  ths = threshold / 32;  break;
        case RANGE_8G:  ths = threshold / 62;  break;
        case RANGE_16G: ths = threshold / 186; break;
        default:        ths = threshold / 16;  break;
    }

    if (!writeReg(INT2_THS, ths))
        return false;

    /**
     * Duration = 0 triggers immediately
     */
    if (!writeReg(INT2_DURATION, 0))
        return false;

    /**
     * Clear any pending interrupt by reading INT2_SRC
     */
    readReg(INT2_SRC, val);

    /**
     * Enable interrupt on X/Y/Z high threshold
     *
     * - OR logic enabled (no 6D mode)
     */
    constexpr uint8_t INT1_ZYX = 0x26;
    if (!writeReg(INT1_CFG, INT1_ZYX))
        return false;

    return true;
}

/**
 * Read interrupt source status for INT1
 *
 * Inputs:
 * - status: reference stores INT1_SRC register value
 *
 * Return:
 * - true  : status read successfully
 * - false : bus read failed
 *
 * Notes:
 * - Reading INT1_SRC clears interrupt latch (if latch enabled)
 */
bool LIS3DH_7Semi::readInt1Status(uint8_t &status)
{
    return readReg(INT1_SRC, status);
}

/**
 * Read interrupt source status for INT2
 *
 * Inputs:
 * - status: reference stores INT2_SRC register value
 *
 * Return:
 * - true  : status read successfully
 * - false : bus read failed
 */
bool LIS3DH_7Semi::readInt2Status(uint8_t &status)
{
    return readReg(INT2_SRC, status);
}

/**
 * Clear INT1 interrupt
 *
 * Inputs:
 * - none
 *
 * Return:
 * - true  : interrupt cleared successfully
 * - false : read failed
 *
 * Notes:
 * - Clearing is done by reading INT1_SRC register
 */
bool LIS3DH_7Semi::clearInt1()
{
    uint8_t status;
    return readInt1Status(status);
}


/**
 * Clear INT2 interrupt
 *
 * Inputs:
 * - none
 *
 * Return:
 * - true  : interrupt cleared successfully
 * - false : read failed
 *
 * Notes:
 * - Clearing is done by reading INT2_SRC register
 */
bool LIS3DH_7Semi::clearInt2()
{
    uint8_t status;
    return readInt2Status(status);
}