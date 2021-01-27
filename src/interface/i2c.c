#include "interface/i2c.h"
#include <stdint.h>
#include "pigpio.h"

i2c_device_t i2cInitialize(struct i2c_settings_t* settings) {
    int8_t device;
    device = i2cOpen(settings->bus_number, settings->address, 0x00);
    if (device >= 0) {
        return (i2c_device_t)device;
    } else {
        return 0;
    }
}

void i2cFinalize(i2c_device_t device) {
    i2cClose(device);
}

void i2cWrite(i2c_device_t device, uint8_t* tx_buffer, uint16_t length) {
    i2cWriteDevice(device, (char*)tx_buffer, length);
}

void i2cRead(i2c_device_t device, uint8_t* rx_buffer, uint16_t length) {
    i2cReadDevice(device, (char*)rx_buffer, length);
}
