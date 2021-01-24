#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct i2c_settings_t {
    uint8_t bus_number;
    uint8_t address;
};

typedef uint8_t i2c_device_t;

i2c_device_t i2cInitialize(struct i2c_settings_t* settings);
void i2cFinalize(i2c_device_t device);

void i2cWrite(i2c_device_t device, uint8_t* tx_buffer, uint16_t length);
void i2cRead(i2c_device_t device, uint8_t* rx_buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif
