#include "bme280.h"

#include <stdint.h>
#include <stddef.h>

#include "spi.h"

static uint8_t const bme280_temp_pres_calib_data_addr = 0x88;
static uint8_t const bme280_hum1_calib_data_addr = 0xA1;
static uint8_t const bme280_hum_calib_data_addr = 0xE1;
static uint8_t const bme280_ctrl_meas_addr = 0xF4;
static uint8_t const bme280_ctrl_hum_addr = 0xF2;
static uint8_t const bme280_config_addr = 0xF5;
static uint8_t const bme280_data_addr = 0xF7;

static uint8_t const bme280_calib_data_len = 32;
static uint8_t const bme280_temp_pres_calib_data_len = 24;
static uint8_t const bme280_hum1_calib_data_len = 1;
static uint8_t const bme280_hum_calib_data_len = 7;
static uint8_t const bme280_data_len = 8;

#ifdef BME280_FLOAT_ENABLE

static double const bme280_temperature_max = 85.0;
static double const bme280_temperature_min = -40.0;
static double const bme280_pressure_max = 1100.0;
static double const bme280_pressure_min = 300.0;
static double const bme280_humidity_max = 100.0;
static double const bme280_humidity_min = 0.0;

#else

static int32_t const bme280_temperature_max = 8500;
static int32_t const bme280_temperature_min = -4000;
static uint32_t const bme280_pressure_max = 110000;
static uint32_t const bme280_pressure_min = 30000;
static uint32_t const bme280_humidity_max = 102400;

#endif

static void bme280ReadCalibData(struct bme280_t* device);

static void bme280SetReg(spi_device_t device, uint8_t address, uint8_t data);
static void bme280GetReg(spi_device_t device, uint8_t address, uint8_t* data,
                         uint8_t len);

void bme280Initialize(struct bme280_t* device, spi_device_t spi_device) {
    device->spi_device = spi_device;
    struct bme280_settings_t settings = {
        .mode = bme280_mode_sleep,
        .osr_temp = bme280_osr_temp_x1,
        .osr_pres = bme280_osr_pres_x1,
        .osr_hum = bme280_osr_hum_x1,
        .standby = bme280_standbytime_1000s,
        .filter = bme280_filter_off,
    };
    bme280SetDeviceSettings(device, &settings);
    bme280ReadCalibData(device);
}

void bme280SetDeviceSettings(struct bme280_t* device,
                             struct bme280_settings_t* settings) {
    device->settings.mode = settings->mode;
    device->settings.osr_temp = settings->osr_temp;
    device->settings.osr_pres = settings->osr_pres;
    device->settings.osr_hum = settings->osr_hum;
    device->settings.standby = settings->standby;
    device->settings.filter = settings->filter;
    uint8_t config_reg = settings->standby | settings->filter;
    uint8_t ctrl_meas_reg = settings->osr_temp | settings->osr_pres | settings->mode;
    uint8_t ctrl_hum_reg = settings->osr_hum;
    bme280SetReg(device->spi_device, bme280_config_addr, config_reg);
    bme280SetReg(device->spi_device, bme280_ctrl_meas_addr, ctrl_meas_reg);
    bme280SetReg(device->spi_device, bme280_ctrl_hum_addr, ctrl_hum_reg);
}

void bme280ReadDeviceData(struct bme280_t* device,
                          struct bme280_data_t* data) {
    uint8_t data_reg[bme280_data_len];
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;
    bme280GetReg(device->spi_device, bme280_data_addr, data_reg, bme280_data_len);
    data_msb = data_reg[0] << 12;
    data_lsb = data_reg[1] << 4;
    data_xlsb = (data_reg[2] & 0xF0) >> 4;
    data->pressure = data_msb | data_lsb | data_xlsb;
    data_msb = data_reg[3] << 12;
    data_lsb = data_reg[4] << 4;
    data_xlsb = (data_reg[5] & 0xF0) >> 4;
    data->temperature = data_msb | data_lsb | data_xlsb;
    data_msb = data_reg[6] << 8;
    data_lsb = data_reg[7];
    data->humidity = data_msb | data_lsb;
}

#ifdef BME280_FLOAT_ENABLE

double bme280CalculateTemperature(struct bme280_t* device, struct bme280_data_t* data) {
    double var1;
    double var2;
    double temperature;
    var1 = ((double)data->temperature) / 16384.0 -
           ((double)device->calib_data.dig_t1) / 1024.0;
    var1 = var1 * ((double)device->calib_data.dig_t2);
    var2 = (((double)data->temperature) / 131072.0 -
            ((double)device->calib_data.dig_t1) / 8192.0);
    var2 = (var2 * var2) * ((double)device->calib_data.dig_t3);
    device->calib_data.t_fine = var1 + var2;
    temperature = (var1 + var2) / 5120.0;
    if (temperature > bme280_temperature_max) {
        temperature = bme280_temperature_max;
    } else if (temperature < bme280_temperature_min) {
        temperature = bme280_temperature_min;
    }
    return temperature;
}

double bme280CalculatePressure(struct bme280_t* device, struct bme280_data_t* data) {
    double var1;
    double var2;
    double var3;
    double pressure;
    var1 = ((double)device->calib_data.t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)device->calib_data.dig_p6) / 32768.0;
    var2 = var2 + var1 * ((double)device->calib_data.dig_p5) * 2.0;
    var2 = (var2 / 4.0) + (((double)device->calib_data.dig_p4) * 65536.0);
    var3 = ((double)device->calib_data.dig_p3) * var1 * var1 / 524288.0;
    var1 = (var3 + ((double)device->calib_data.dig_p2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)device->calib_data.dig_p1);
    if (var1 > 0.0) {
        pressure = 1048576.0 - (double)data->pressure;
        pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
        var1 =
            ((double)device->calib_data.dig_p9) * pressure * pressure / 2147483648.0;
        var2 = pressure * ((double)device->calib_data.dig_p8) / 32768.0;
        pressure =
            pressure + (var1 + var2 + ((double)device->calib_data.dig_p7)) / 16.0;
        pressure /= 100.0;
        if (pressure > bme280_pressure_max) {
            pressure = bme280_pressure_max;
        } else if (pressure < bme280_pressure_min) {
            pressure = bme280_pressure_min;
        }
    } else {
        pressure = bme280_pressure_min;
    }
    return pressure;
}

double bme280CalculateHumidity(struct bme280_t* device, struct bme280_data_t* data) {
    double var1;
    double var2;
    double var3;
    double var4;
    double var5;
    double var6;
    double humidity;
    var1 = ((double)device->calib_data.t_fine) - 76800.0;
    var2 = (((double)device->calib_data.dig_h4) * 64.0 +
            (((double)device->calib_data.dig_h5) / 16384.0) * var1);
    var3 = data->humidity - var2;
    var4 = ((double)device->calib_data.dig_h2) / 65536.0;
    var5 = (1.0 + (((double)device->calib_data.dig_h3) / 67108864.0) * var1);
    var6 = 1.0 + (((double)device->calib_data.dig_h6) / 67108864.0) * var1 * var5;
    var6 = var3 * var4 * (var5 * var6);
    humidity = var6 * (1.0 - ((double)device->calib_data.dig_h1) * var6 / 524288.0);
    if (humidity > bme280_humidity_max) {
        humidity = bme280_humidity_max;
    } else if (humidity < bme280_humidity_min) {
        humidity = bme280_humidity_min;
    }
    return humidity;
}

#else

int32_t bme280CalculateTemperature(struct bme280_t* device, struct bme280_data_t* data) {
    int32_t var1;
    int32_t var2;
    int32_t temperature;
    var1 =
        (int32_t)((data->temperature / 8) - ((int32_t)device->calib_data.dig_t1 * 2));
    var1 = (var1 * ((int32_t)device->calib_data.dig_t2)) / 2048;
    var2 = (int32_t)((data->temperature / 16) - ((int32_t)device->calib_data.dig_t1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)device->calib_data.dig_t3)) / 16384;
    device->calib_data.t_fine = var1 + var2;
    temperature = (device->calib_data.t_fine * 5 + 128) / 256;
    if (temperature > bme280_temperature_max) {
        temperature = bme280_temperature_max;
    } else if (temperature < bme280_temperature_min) {
        temperature = bme280_temperature_min;
    }
    return temperature;
}

uint32_t bme280CalculatePressure(struct bme280_t* device, struct bme280_data_t* data) {
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    uint32_t var5;
    uint32_t pressure;
    var1 = (((int32_t)device->calib_data.t_fine) / 2) - (int32_t)64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)device->calib_data.dig_p6);
    var2 = var2 + ((var1 * ((int32_t)device->calib_data.dig_p5)) * 2);
    var2 = (var2 / 4) + (((int32_t)device->calib_data.dig_p4) * 65536);
    var3 = (device->calib_data.dig_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
    var4 = (((int32_t)device->calib_data.dig_p2) * var1) / 2;
    var1 = (var3 + var4) / 262144;
    var1 = (((32768 + var1)) * ((int32_t)device->calib_data.dig_p1)) / 32768;
    if (var1) {
        var5 = (uint32_t)((uint32_t)1048576) - data->pressure;
        pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;
        if (pressure < 0x80000000) {
            pressure = (pressure << 1) / ((uint32_t)var1);
        } else {
            pressure = (pressure / (uint32_t)var1) * 2;
        }
        var1 = (((int32_t)device->calib_data.dig_p9) *
                ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) /
               4096;
        var2 =
            (((int32_t)(pressure / 4)) * ((int32_t)device->calib_data.dig_p8)) / 8192;
        pressure = (uint32_t)((int32_t)pressure +
                              ((var1 + var2 + device->calib_data.dig_p7) / 16));
        if (pressure > bme280_pressure_max) {
            pressure = bme280_pressure_max;
        } else if (pressure < bme280_pressure_min) {
            pressure = bme280_pressure_min;
        }
    } else {
        pressure = bme280_pressure_min;
    }
    return pressure;
}

uint32_t bme280CalculateHumidity(struct bme280_t* device, struct bme280_data_t* data) {
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    uint32_t humidity;
    var1 = device->calib_data.t_fine - ((int32_t)76800);
    var2 = (int32_t)(data->humidity * 16384);
    var3 = (int32_t)(((int32_t)device->calib_data.dig_h4) * 1048576);
    var4 = ((int32_t)device->calib_data.dig_h5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)device->calib_data.dig_h6)) / 1024;
    var3 = (var1 * ((int32_t)device->calib_data.dig_h3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)device->calib_data.dig_h2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)device->calib_data.dig_h1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    humidity = (uint32_t)(var5 / 4096);
    if (humidity > bme280_humidity_max) {
        humidity = bme280_humidity_max;
    }
    return humidity;
}

#endif

static void bme280ReadCalibData(struct bme280_t* device) {
    struct bme280_calib_data_t* data = &device->calib_data;
    uint8_t reg_data[bme280_calib_data_len];
    uint16_t dig_h4_lsb;
    uint16_t dig_h4_msb;
    uint16_t dig_h5_lsb;
    uint16_t dig_h5_msb;
    bme280GetReg(device->spi_device, bme280_temp_pres_calib_data_addr,
                 &reg_data[0], bme280_temp_pres_calib_data_len);
    bme280GetReg(device->spi_device, bme280_hum1_calib_data_addr,
                 &reg_data[24], bme280_hum1_calib_data_len);
    bme280GetReg(device->spi_device, bme280_hum_calib_data_addr, &reg_data[25],
                 bme280_hum_calib_data_len);
    data->dig_t1 = reg_data[1] << 8 | reg_data[0];
    data->dig_t2 = (int16_t)(reg_data[3] << 8 | reg_data[2]);
    data->dig_t3 = (int16_t)(reg_data[5] << 8 | reg_data[4]);
    data->dig_p1 = reg_data[7] << 8 | reg_data[6];
    data->dig_p2 = (int16_t)(reg_data[9] << 8 | reg_data[8]);
    data->dig_p3 = (int16_t)(reg_data[11] << 8 | reg_data[10]);
    data->dig_p4 = (int16_t)(reg_data[13] << 8 | reg_data[12]);
    data->dig_p5 = (int16_t)(reg_data[15] << 8 | reg_data[14]);
    data->dig_p6 = (int16_t)(reg_data[17] << 8 | reg_data[16]);
    data->dig_p7 = (int16_t)(reg_data[19] << 8 | reg_data[18]);
    data->dig_p8 = (int16_t)(reg_data[21] << 8 | reg_data[20]);
    data->dig_p9 = (int16_t)(reg_data[23] << 8 | reg_data[22]);
    data->dig_h1 = reg_data[24];
    data->dig_h2 = (int16_t)(reg_data[26] << 8 | reg_data[25]);
    data->dig_h3 = reg_data[27];
    dig_h4_msb = reg_data[28] << 4;
    dig_h4_lsb = reg_data[29] & 0x0F;
    data->dig_h4 = (int16_t)(dig_h4_msb | dig_h4_lsb);
    dig_h5_msb = reg_data[30] << 4;
    dig_h5_lsb = (reg_data[29] & 0xF0) >> 4;
    data->dig_h5 = (int16_t)(dig_h5_msb | dig_h5_lsb);
    data->dig_h6 = (int8_t)reg_data[31];
}

static void bme280SetReg(spi_device_t device, uint8_t address, uint8_t data) {
    uint8_t buf[2];
    buf[0] = address & 0x7F;
    buf[1] = data;
    spiTransfer(device, buf, NULL, 2);
}

static void bme280GetReg(spi_device_t device, uint8_t address, uint8_t* data,
                         uint8_t len) {
    uint8_t buf[len + 1];
    uint8_t tmp[len + 1];
    buf[0] = address | 0x80;
    for (uint8_t i = 0; i < len; i++) {
        buf[i + 1] = 0x00;
    }
    spiTransfer(device, buf, tmp, len + 1);
    for (uint8_t i = 0; i < len; i++) {
        data[i] = tmp[i + 1];
    }
}
