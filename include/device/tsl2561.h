#ifndef __TSL2561_H__
#define __TSL2561_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "interface/i2c.h"

extern uint8_t const tsl2561_addr_low;
extern uint8_t const tsl2561_addr_float;
extern uint8_t const tsl2561_addr_high;

enum tsl2561_gain_t {
    tsl2561_gain_1x = 0x00,
    tsl2561_gain_16x = 0x10,
};

enum tsl2561_integral_t {
    tsl2561_integral_13ms = 0x00,
    tsl2561_integral_101ms = 0x01,
    tsl2561_integral_402ms = 0x02,
};

struct tsl2561_settings_t {
    enum tsl2561_gain_t gain;
    enum tsl2561_integral_t integral;
};

struct tsl2561_data_t {
    uint16_t channel_0;
    uint16_t channel_1;
};

struct tsl2561_t {
    i2c_device_t i2c_device;
    struct tsl2561_settings_t settings;
};

void tsl2561Initialize(struct tsl2561_t* device, i2c_device_t i2c_device);

void tsl2561SetDeviceSettings(struct tsl2561_t* device,
                              struct tsl2561_settings_t* settings);

void tsl2561ReadDeviceData(struct tsl2561_t* device,
                           struct tsl2561_data_t* data);

#ifdef TSL2561_FLOAT_ENABLE

double tsl2561CalculateIlluminance(struct tsl2561_t* device,
                                   struct tsl2561_data_t* data);

#else

uint32_t tsl2561CalculateIlluminance(struct tsl2561_t* device,
                                     struct tsl2561_data_t* data);

#endif

#ifdef __cplusplus
}
#endif

#endif