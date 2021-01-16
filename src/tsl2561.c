#include "tsl2561.h"

#ifdef TSL2561_FLOAT_ENABLE
#include <math.h>
#endif
#include <stdint.h>

#include "i2c.h"

#include <unistd.h>
void delay(uint16_t ms) {
    usleep(ms * 1000);
}

uint8_t const tsl2561_addr_low = 0x29;
uint8_t const tsl2561_addr_float = 0x39;
uint8_t const tsl2561_addr_high = 0x49;

static uint8_t const tsl2561_control_addr = 0x80;
static uint8_t const tsl2561_timing_addr = 0x81;
static uint8_t const tsl2561_id_addr = 0x8A;
static uint8_t const tsl2561_channel0_data_addr = 0x8C;
static uint8_t const tsl2561_channel1_data_addr = 0x8E;

static uint8_t const tsl2561_control_power_off = 0x00;
static uint8_t const tsl2561_control_power_on = 0x03;

uint8_t const tsl2561_lux_luxscale = 14;
uint8_t const tsl2561_lux_ratioscale = 9;
uint8_t const tsl2561_lux_chscale = 10;

uint16_t const tsl2561_lux_chscale_tint0 = 0x7517;
uint16_t const tsl2561_lux_chscale_tint1 = 0x0FE7;

static void tsl2561SetReg(i2c_device_t i2c_device, uint8_t address,
                          uint8_t data);
static void tsl2561GetReg(i2c_device_t i2c_device, uint8_t address,
                          uint8_t* data, uint8_t length);

static void tsl2561PowerOff(i2c_device_t i2c_device);
static void tsl2561PowerOn(i2c_device_t i2c_device);

void tsl2561Initialize(struct tsl2561_t* device, i2c_device_t i2c_device) {
    device->i2c_device = i2c_device;
    struct tsl2561_settings_t settings = {
        .gain = tsl2561_gain_16x,
        .integral = tsl2561_integral_402ms,
    };
    tsl2561SetDeviceSettings(device, &settings);
}

void tsl2561SetDeviceSettings(struct tsl2561_t* device,
                              struct tsl2561_settings_t* settings) {
    tsl2561PowerOn(device->i2c_device);
    device->settings.gain = settings->gain;
    device->settings.integral = settings->integral;
    tsl2561SetReg(device->i2c_device, tsl2561_timing_addr,
                  device->settings.gain | device->settings.integral);
    tsl2561PowerOff(device->i2c_device);
}

void tsl2561ReadDeviceData(struct tsl2561_t* device,
                           struct tsl2561_data_t* data) {
    tsl2561PowerOn(device->i2c_device);
    switch (device->settings.integral) {
    case tsl2561_integral_13ms:
        delay(14);
        break;
    case tsl2561_integral_101ms:
        delay(102);
        break;
    case tsl2561_integral_402ms:
        delay(403);
        break;
    }
    uint8_t ch0_buf[2];
    uint8_t ch1_buf[2];
    tsl2561GetReg(device->i2c_device, tsl2561_channel0_data_addr, ch0_buf, 2);
    tsl2561GetReg(device->i2c_device, tsl2561_channel1_data_addr, ch1_buf, 2);
    data->channel_0 = ch0_buf[1] << 8 | ch0_buf[0];
    data->channel_1 = ch1_buf[1] << 8 | ch1_buf[0];
    tsl2561PowerOff(device->i2c_device);
}

#ifdef TSL2561_FLOAT_ENABLE

double tsl2561CalculateIlluminance(struct tsl2561_t* device,
                                   struct tsl2561_data_t* data) {
    double illuminance;
    if (data->channel_0 != 0) {
        double result = (double)data->channel_1 / (double)data->channel_0;
#ifdef TSL2561_PACKAGE_CS
        if (result <= 0.52) {
            illuminance = 0.0315 * data->channel_0 -
                          0.0593 * data->channel_0 * pow(result, 1.4);
        } else if (result <= 0.65) {
            illuminance = 0.0229 * data->channel_0 - 0.0291 * data->channel_1;
        } else if (result <= 0.80) {
            illuminance = 0.0157 * data->channel_0 - 0.0180 * data->channel_1;
        } else if (result <= 1.30) {
            illuminance = 0.00338 * data->channel_0 - 0.00260 * data->channel_1;
        } else {
            illuminance = 0.0;
        }
#else
        if (result <= 0.50) {
            illuminance = 0.0304 * data->channel_0 -
                          0.062 * data->channel_0 * pow(result, 1.4);
        } else if (result <= 0.61) {
            illuminance = 0.0224 * data->channel_0 - 0.031 * data->channel_1;
        } else if (result <= 0.80) {
            illuminance = 0.0128 * data->channel_0 - 0.0153 * data->channel_1;
        } else if (result <= 1.30) {
            illuminance = 0.00146 * data->channel_0 - 0.00112 * data->channel_1;
        } else {
            illuminance = 0.0;
        }
#endif
    } else {
        illuminance = 0.0;
    }
    if (device->settings.gain == tsl2561_gain_1x) {
        illuminance *= 16;
    }
    switch (device->settings.integral) {
    case tsl2561_integral_13ms:
        illuminance *= 402.0 / 13.7;
        break;
    case tsl2561_integral_101ms:
        illuminance *= 402.0 / 101.0;
        break;
    default:
        break;
    }
    return illuminance;
}

#else

uint32_t tsl2561CalculateIlluminance(struct tsl2561_t* device,
                                     struct tsl2561_data_t* data) {
    uint32_t chScale;
    uint32_t channel0;
    uint32_t channel1;
    uint32_t ratio = 0;
    uint16_t b, m;
    uint32_t temp;
    uint32_t illuminance;
    switch (device->settings.integral) {
    case tsl2561_integral_13ms:
        chScale = tsl2561_lux_chscale_tint0;
        break;
    case tsl2561_integral_101ms:
        chScale = tsl2561_lux_chscale_tint1;
        break;
    case tsl2561_integral_402ms:
        chScale = (1 << tsl2561_lux_chscale);
        break;
    }
    if (device->settings.gain == tsl2561_gain_1x) {
        chScale = chScale << 4;
    }
    channel0 = (data->channel_0 * chScale) >> tsl2561_lux_chscale;
    channel1 = (data->channel_1 * chScale) >> tsl2561_lux_chscale;
    if (channel0 != 0) {
        ratio = (channel1 << (tsl2561_lux_ratioscale + 1)) / channel0;
    }
    ratio = (ratio + 1) >> 1;
#ifdef TSL2561_PACKAGE_CS
    if (ratio <= 0x0043) {
        b = 0x0204;
        m = 0x01ad;
    } else if (ratio <= 0x0085) {
        b = 0x0228;
        m = 0x02c1;
    } else if (ratio <= 0x00c8) {
        b = 0x0253;
        m = 0x0363;
    } else if (ratio <= 0x010a) {
        b = 0x0282;
        m = 0x03df;
    } else if (ratio <= 0x014d) {
        b = 0x0177;
        m = 0x01dd;
    } else if (ratio <= 0x019a) {
        b = 0x0101;
        m = 0x0127;
    } else if (ratio <= 0x029a) {
        b = 0x0037;
        m = 0x002b;
    } else if (ratio > 0x029a) {
        b = 0x0000;
        m = 0x0000;
    }
#else
    if (ratio <= 0x0040) {
        b = 0x01f2;
        m = 0x01be;
    } else if (ratio <= 0x0080) {
        b = 0x0214;
        m = 0x02d1;
    } else if (ratio <= 0x00c0) {
        b = 0x023f;
        m = 0x037b;
    } else if (ratio <= 0x0100) {
        b = 0x0270;
        m = 0x03fe;
    } else if (ratio <= 0x0138) {
        b = 0x016f;
        m = 0x01fc;
    } else if (ratio <= 0x019a) {
        b = 0x00d2;
        m = 0x00fb;
    } else if (ratio <= 0x029a) {
        b = 0x0018;
        m = 0x0012;
    } else if (ratio > 0x029a) {
        b = 0x0000;
        m = 0x0000;
    }
#endif
    temp = ((channel0 * b) - (channel1 * m));
    temp += (1 << (tsl2561_lux_luxscale - 1));
    illuminance = temp >> tsl2561_lux_luxscale;
    return illuminance;
}

#endif

static void tsl2561SetReg(i2c_device_t i2c_device, uint8_t address,
                          uint8_t data) {
    uint8_t buf[2];
    buf[0] = address;
    buf[1] = data;
    i2cWrite(i2c_device, buf, 2);
}

static void tsl2561GetReg(i2c_device_t i2c_device, uint8_t address,
                          uint8_t* data, uint8_t length) {
    uint8_t tx_buf[1];
    tx_buf[0] = address;
    i2cWrite(i2c_device, tx_buf, 1);
    i2cRead(i2c_device, data, length);
}

static void tsl2561PowerOff(i2c_device_t i2c_device) {
    tsl2561SetReg(i2c_device, tsl2561_control_addr, tsl2561_control_power_off);
}
static void tsl2561PowerOn(i2c_device_t i2c_device) {
    tsl2561SetReg(i2c_device, tsl2561_control_addr, tsl2561_control_power_on);
    delay(50);
}
