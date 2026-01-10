/**
 * 7Semi LIS3DH Library
 *
 * - Driver for LIS3DH 3-axis accelerometer (STMicroelectronics)
 * - Supports both I2C and SPI communication modes
 * - Provides acceleration, interrupt, ADC, and temperature features
 * - Designed for Arduino-compatible platforms
 *
 * Library Features:
 * - Acceleration output in raw (int16_t) and converted g-units (float)
 * - INT1 and INT2 threshold interrupt configuration
 * - ADC channel reading (ADC1, ADC2, ADC3)
 * - Temperature output using internal sensor (ADC3 based)
 *
 * Notes:
 * - Default I2C address is 0x18 (can also be 0x19 depending on SA0 pin)
 * - SPI mode uses manual CS pin control
 * - WHO_AM_I register must return 0x33 for LIS3DH
 */

#ifndef _7SEMI_LIS3DH_H_
#define _7SEMI_LIS3DH_H_

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>


/**
 * ADC Output Registers (Low/High byte)
 *
 * - ADC1/ADC2/ADC3 output data registers
 * - ADC3 output is also used for temperature output
 */
#define OUT_ADC1_L      0x08
#define OUT_ADC1_H      0x09
#define OUT_ADC2_L      0x0A
#define OUT_ADC2_H      0x0B
#define OUT_ADC3_L      0x0C
#define OUT_ADC3_H      0x0D


/**
 * LIS3DH Register Map
 *
 * - Register addresses used for identification, configuration,
 *   data output, interrupts, ADC, and temperature features.
 */
#define WHO_AM_I        0x0F
#define TEMP_CFG_REG    0x1F
#define CTRL_REG0       0x1E
#define CTRL_REG1       0x20
#define CTRL_REG2       0x21
#define CTRL_REG3       0x22
#define CTRL_REG4       0x23
#define CTRL_REG5       0x24
#define CTRL_REG6       0x25



/**
 * Accelerometer Output Registers (Low byte)
 *
 * - X, Y, Z axis acceleration data output
 * - Each axis has low and high byte registers
 */
#define OUT_X_L         0x28
#define OUT_Y_L         0x2A
#define OUT_Z_L         0x2C

/**
 * Interrupt 1 Registers
 *
 * - Configure, read status, set threshold and duration
 */
#define INT1_CFG        0x30
#define INT1_SRC        0x31
#define INT1_THS        0x32
#define INT1_DURATION   0x33

/**
 * Interrupt 2 Registers
 *
 * - Configure, read status, set threshold and duration
 */
#define INT2_CFG        0x34
#define INT2_SRC        0x35
#define INT2_THS        0x36
#define INT2_DURATION   0x37

/**
 * Accelerometer measurement range options
 *
 * - Total available options: 4
 * - These values match the FS bits written into CTRL_REG4
 *
 * Options:
 * - RANGE_2G  : ±2g
 * - RANGE_4G  : ±4g
 * - RANGE_8G  : ±8g
 * - RANGE_16G : ±16g
 */
#define RANGE_2G        0x00
#define RANGE_4G        0x01
#define RANGE_8G        0x02
#define RANGE_16G       0x03

/**
 * Output Data Rate (ODR) options for LIS3DH
 *
 * - Total available options: 10 commonly supported values
 * - These values map to CTRL_REG1 ODR bits [7:4]
 *
 * Options:
 * - ODR_POWER_DOWN : Power-down mode
 * - ODR_1HZ        : 1 Hz
 * - ODR_10HZ       : 10 Hz
 * - ODR_25HZ       : 25 Hz
 * - ODR_50HZ       : 50 Hz
 * - ODR_100HZ      : 100 Hz
 * - ODR_200HZ      : 200 Hz
 * - ODR_400HZ      : 400 Hz
 * - ODR_1600HZ_LP  : 1.6 kHz (low-power mode only)
 * - ODR_1344HZ_N   : 1.344 kHz (normal) / 5.376 kHz (low-power)
 *
 * Notes:
 * - Values 0x0A to 0x0F are typically reserved
 * - Recommended stable values: 1Hz to 400Hz
 */
#define ODR_POWER_DOWN   0x00
#define ODR_1HZ          0x01
#define ODR_10HZ         0x02
#define ODR_25HZ         0x03
#define ODR_50HZ         0x04
#define ODR_100HZ        0x05
#define ODR_200HZ        0x06
#define ODR_400HZ        0x07
#define ODR_1600HZ_LP    0x08
#define ODR_1344HZ_N     0x09


/**
 * LIS3DH_7Semi Class
 *
 * - Handles communication over I2C or SPI
 * - Supports acceleration readouts in raw and g-units
 * - Includes interrupt, ADC and temperature support
 */
class LIS3DH_7Semi
{
public:

    LIS3DH_7Semi();

    /**
     * Initialize LIS3DH over I2C
     *
     * Inputs:
     * - wire: Wire interface object
     * - i2c_address: sensor I2C address
     * - i2c_sda: optional SDA pin 
     * - i2c_scl: optional SCL pin 
     * - i2c_speed: I2C clock speed in Hz
     *
     * Return:
     * - true  : sensor detected and initialized
     * - false : sensor not detected or bus failed
     */
    bool begin(TwoWire &wire = Wire, uint8_t i2c_address = 0x18,
               uint8_t i2c_sda = 0xFF, uint8_t i2c_scl = 0xFF,
               uint32_t i2c_speed = 400000);

/**
     * Initialize LIS3DH over SPI
     *
     * Inputs:
     * - spiPort: SPI interface object 
     * - spi_cs: chip select pin
     * - spi_sck: optional SCK pin 
     * - spi_miso: optional MISO pin 
     * - spi_mosi: optional MOSI pin 
     * - spi_speed: SPI clock speed in Hz 
     *
     * Return:
     * - true  : sensor detected and initialized
     * - false : sensor not detected or bus failed
     */
    bool begin(SPIClass &spiPort, uint8_t spi_cs = 10,
               uint8_t spi_sck = 0xFF, uint8_t spi_miso = 0xFF,
               uint8_t spi_mosi = 0xFF, uint32_t spi_speed = 1000000);

    /**
     * Read device ID (WHO_AM_I)
     *
     * Inputs:
     * - id: reference that stores WHO_AM_I value
     *
     * Return:
     * - true  : read successful
     * - false : read failed
     *
     * Notes:
     * - LIS3DH should return 0x33
     */
    bool readID(uint8_t &id);

    /**
     * Set accelerometer measurement range
     *
     * Inputs:
     * - scale: Full Scale selection constant
     *
     * - RANGE_2G  (±2g)
     * - RANGE_4G  (±4g)
     * - RANGE_8G  (±8g)
     * - RANGE_16G (±16g)
     *
     * Return:
     * - true  : scale updated successfully
     * - false : invalid scale or register write failed
     */
    bool setScale(uint8_t scale);

    /**
     * Set accelerometer output data rate (ODR)
     *
     * Inputs:
     * - dataRate: output data rate selection value
     *
     * Total available options: 10 commonly supported values
     *
     * Available options (ODR bits in CTRL_REG1 [7:4]):
     * - ODR_POWER_DOWN (0x00) : Power-down mode
     * - ODR_1HZ        (0x01) : 1 Hz
     * - ODR_10HZ       (0x02) : 10 Hz
     * - ODR_25HZ       (0x03) : 25 Hz
     * - ODR_50HZ       (0x04) : 50 Hz
     * - ODR_100HZ      (0x05) : 100 Hz
     * - ODR_200HZ      (0x06) : 200 Hz
     * - ODR_400HZ      (0x07) : 400 Hz
     * - ODR_1600HZ_LP  (0x08) : 1.6 kHz (low-power mode only)
     * - ODR_1344HZ_N   (0x09) : 1.344 kHz (normal mode) / 5.376 kHz (low-power)
     *
     * Notes:
     * - Values 0x0A to 0x0F are reserved in most datasheet versions
     * - Recommended stable values: 1 Hz to 400 Hz
     * - This function updates only the ODR bits and keeps XYZ enable bits intact
     *
     * Return:
     * - true  : output data rate updated successfully
     * - false : invalid dataRate or register write/read failed
     */

    bool setDataRate(uint8_t dataRate);

    /**
     * Enable or disable High-Resolution (HR) mode
     *
     * Inputs:
     * - enable : true  -> enable High-Resolution mode
     *            false -> disable High-Resolution mode
     *
     * Return:
     * - true  : CTRL_REG4 updated successfully
     * - false : Communication error
     *
     * Notes:
     * - High-Resolution mode is controlled by CTRL_REG4 bit 3 (HR)
     * - HR mode provides 12-bit acceleration data (left-justified)
     * - HR mode increases current consumption
     * - HR mode DISABLES ADC and temperature sensor updates
     * - Do NOT enable HR mode if ADC or temperature readings are required
     */

     bool setHighResolution(bool enable);

    /**
     * Read accelerometer output (aligned values)
     *
     * Inputs:
     * - x: reference stores aligned X-axis value
     * - y: reference stores aligned Y-axis value
     * - z: reference stores aligned Z-axis value
     *
     * Return:
     * - true  : read successful
     * - false : bus read failed
     *
     * Notes:
     * - Automatically applies shifts depending on LP/HR/Normal mode
     */
    bool readAccel(int16_t &x, int16_t &y, int16_t &z);
   
    
    /**
     * Read accelerometer output (raw register values)
     *
     * Inputs:
     * - x: reference stores raw X-axis register value
     * - y: reference stores raw Y-axis register value
     * - z: reference stores raw Z-axis register value
     *
     * Return:
     * - true  : read successful
     * - false : bus read failed
     *
     * Notes:
     * - No shifting is applied (useful for debugging)
     */
    bool readAccelRaw(int16_t &x, int16_t &y, int16_t &z);
    
    /**
     * Read acceleration in g units for each axis
     *
     * Inputs:
     * - x / y / z: reference stores converted value in g
     *
     * Return:
     * - true  : read + conversion successful
     * - false : sensor read failed
     */
    bool getAccelerationX(float &x);
    bool getAccelerationY(float &y);
    bool getAccelerationZ(float &z);

    /**
     * Read single ADC channel value
     *
     * Inputs:
     * - channel: ADC channel number
     *
     * Total available channels: 3
     * Available options:
     * - 1 : ADC1 (OUT_ADC1_L/H)
     * - 2 : ADC2 (OUT_ADC2_L/H)
     * - 3 : ADC3 (OUT_ADC3_L/H)
     *
     * Output:
     * - adcValue: reference stores 16-bit ADC value
     *
     * Return:
     * - true  : read successful
     * - false : invalid channel or bus read failed
     */
    bool readADC_CH(uint8_t channel, int16_t &adcValue);

    /**
     * Read all ADC channels at once
     *
     * Outputs:
     * - adcX: ADC1 output value
     * - adcY: ADC2 output value
     * - adcZ: ADC3 output value
     *
     * Return:
     * - true  : read successful
     * - false : bus read failed
     */
    bool readADC(int16_t &adcX, int16_t &adcY, int16_t &adcZ);

    /**
     * Enable or disable ADC block
     *
     * Inputs:
     * - enable:
     *   - true  : enable ADC feature block
     *   - false : disable ADC feature block
     *
     * Return:
     * - true  : updated successfully
     * - false : bus read/write failed
     */
    bool enableADC(bool enable);

    /**
     * Enable or disable temperature output
     *
     * Inputs:
     * - enable:
     *   - true  : enable temperature output on ADC3
     *   - false : disable temperature output
     *
     * Return:
     * - true  : updated successfully
     * - false : bus read/write failed
     */
    bool enableTemperature(bool enable);

    /**
     * Read temperature output
     *
     * Output:
     * - temperature: reference stores temperature in Celsius
     *
     * Return:
     * - true  : read successful
     * - false : temperature not enabled or bus failed
     *
     * Notes:
     * - Temperature output comes from ADC3 registers
     * - Conversion is approximate unless calibrated
     */
    bool getTemperature(float &temperature);

    /**
     * Enable and configure INT1 interrupt
     *
     * Inputs:
     * - active_high: polarity selection
     *   - true  : interrupt pin active HIGH
     *   - false : interrupt pin active LOW
     * - threshold_mg: threshold in milli-g (mg)
     *
     * Return:
     * - true  : interrupt enabled successfully
     * - false : configuration failed
     */
    bool enableINT1(bool active_high, uint16_t threshold_mg);

    /**
     * Enable and configure INT2 interrupt
     *
     * Inputs:
     * - active_high: polarity selection
     *   - true  : interrupt pin active HIGH
     *   - false : interrupt pin active LOW
     * - threshold: threshold value (mg)
     *
     * Return:
     * - true  : interrupt enabled successfully
     * - false : configuration failed
     */
    bool enableINT2(bool active_high, uint8_t threshold);

    /**
     * Read interrupt status
     *
     * Inputs:
     * - status: reference stores interrupt source flags
     *
     * Return:
     * - true  : status read successful
     * - false : bus read failed
     */
    bool readInt1Status(uint8_t &status);
    bool readInt2Status(uint8_t &status);

    /**
     * Clear interrupts by reading source registers
     *
     * Return:
     * - true  : interrupt cleared successfully
     * - false : bus read failed
     */
    bool clearInt1();
    bool clearInt2();

    

private:
    /**
     * Communication type selection
     *
     * - SPI_bus: SPI communication mode
     * - I2C_bus: I2C communication mode
     *
     * Notes:
     * - Values assigned explicitly to allow stable selection
     */
    enum bus_type
    {
        SPI_bus = 0,
        I2C_bus = 1
    } bus;

    /**
     * Cached configuration values
     *
     * - acc_scale: current selected accelerometer range
     * - data_rate: last selected output data rate
     */
    uint8_t acc_scale;
    uint8_t data_rate;

    /**
     * Shared communication handles
     *
     * - i2c and spi are static so hardware peripherals are shared
     * - cs_pin and sensor_address belong to the selected bus
     */
    static TwoWire *i2c;
    static SPIClass *spi;
    static uint8_t cs_pin;
    static uint8_t sensor_address;

    /**
     * Bus speed cache
     *
     * - Stores last configured I2C/SPI bus speed
     */
    uint32_t speed;

    /**
     * I2C read/write helpers
     *
     * - Used internally by readReg() and writeReg()
     */
    bool i2c_read(uint8_t reg, uint8_t &value);
    bool i2c_write(uint8_t reg, uint8_t value);

    /**
     * SPI read/write helpers
     *
     * - Used internally by readReg() and writeReg()
     */
    bool spi_read(uint8_t reg, uint8_t &value);
    bool spi_write(uint8_t reg, uint8_t value);

    /**
     * Abstracted register access functions
     *
     * - Automatically routes register access to active bus type
     */
    bool readReg(uint8_t reg, uint8_t &val);
    bool writeReg(uint8_t reg, uint8_t val);

    /**
     * Convert raw acceleration data into g-units
     *
     * Inputs:
     * - raw: signed raw axis value
     *
     * Return:
     * - acceleration in g units
     *
     * Notes:
     * - Conversion depends on current range (scale) and mode (LP/HR)
     */
    float rawToG(int16_t raw);
};

#endif // _7SEMI_LIS3DH_H_
