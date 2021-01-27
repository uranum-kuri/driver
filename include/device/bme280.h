#ifndef __BME280_H__
#define __BME280_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "interface/spi.h"

enum bme280_osr_temp_t {
    bme280_osr_temp_skip = 0x00,
    bme280_osr_temp_x1 = 0x20,
    bme280_osr_temp_x2 = 0x40,
    bme280_osr_temp_x4 = 0x60,
    bme280_osr_temp_x8 = 0x80,
    bme280_osr_temp_x16 = 0xA0,
};

enum bme280_osr_pres_t {
    bme280_osr_pres_skip = 0x00,
    bme280_osr_pres_x1 = 0x04,
    bme280_osr_pres_x2 = 0x08,
    bme280_osr_pres_x4 = 0x0C,
    bme280_osr_pres_x8 = 0x10,
    bme280_osr_pres_x16 = 0x14,
};

enum bme280_osr_hum_t {
    bme280_osr_hum_skip = 0x00,
    bme280_osr_hum_x1 = 0x01,
    bme280_osr_hum_x2 = 0x02,
    bme280_osr_hum_x4 = 0x03,
    bme280_osr_hum_x8 = 0x04,
    bme280_osr_hum_x16 = 0x05,
};

enum bme280_standbytime_t {
    bme280_standbytime_1s = 0x00,
    bme280_standbytime_62s = 0x20,
    bme280_standbytime_125s = 0x40,
    bme280_standbytime_250s = 0x60,
    bme280_standbytime_500s = 0x80,
    bme280_standbytime_1000s = 0xA0,
    bme280_standbytime_10s = 0xC0,
    bme280_standbytime_20s = 0xE0,
};

enum bme280_filter_t {
    bme280_filter_off = 0x00,
    bme280_filter_coeff_2 = 0x04,
    bme280_filter_coeff_4 = 0x08,
    bme280_filter_coeff_8 = 0x0C,
    bme280_filter_coeff_16 = 0x10,
};

enum bme280_mode_t {
    bme280_mode_sleep = 0x00,
    bme280_mode_forced = 0x01,
    bme280_mode_normal = 0x03,
};

struct bme280_settings_t {
    enum bme280_mode_t mode;
    enum bme280_osr_temp_t osr_temp;
    enum bme280_osr_pres_t osr_pres;
    enum bme280_osr_hum_t osr_hum;
    enum bme280_standbytime_t standby;
    enum bme280_filter_t filter;
};

struct bme280_calib_data_t {
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
    uint8_t dig_h1;
    int16_t dig_h2;
    uint8_t dig_h3;
    int16_t dig_h4;
    int16_t dig_h5;
    int8_t dig_h6;
    int32_t t_fine;
};

struct bme280_data_t {
    uint32_t temperature;
    uint32_t pressure;
    uint32_t humidity;
};

struct bme280_t {
    spi_device_t spi_device;
    struct bme280_settings_t settings;
    struct bme280_calib_data_t calib_data;
};

void bme280Initialize(struct bme280_t* device, spi_device_t spi_device);

void bme280SetDeviceSettings(struct bme280_t* device,
                             struct bme280_settings_t* settings);

void bme280ReadDeviceData(struct bme280_t* device, struct bme280_data_t* data);

#ifdef BME280_FLOAT_ENABLE

double bme280CalculateTemperature(struct bme280_t* device,
                                  struct bme280_data_t* data);
double bme280CalculatePressure(struct bme280_t* device,
                               struct bme280_data_t* data);
double bme280CalculateHumidity(struct bme280_t* device,
                               struct bme280_data_t* data);

#else

int32_t bme280CalculateTemperature(struct bme280_t* device,
                                   struct bme280_data_t* data);
uint32_t bme280CalculatePressure(struct bme280_t* device,
                                 struct bme280_data_t* data);
uint32_t bme280CalculateHumidity(struct bme280_t* device,
                                 struct bme280_data_t* data);

#endif

#ifdef __cplusplus
}
#endif

#endif
