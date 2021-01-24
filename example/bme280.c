#include <stdint.h>
#include <stdio.h>

#include "pigpio.h"
#include "interface/spi.h"
#include "device/bme280.h"

int main(void) {
    // Initialize pigpio
    gpioInitialise();

    // Initialize spi device
    struct spi_settings_t spi_settings = {
        .slave_number = 0,
        .clock_speed = 100000,
        .mode_number = 0,
        .active_high = spi_active_high_disable,
    };
    spi_device_t spi_device = spiInitialize(&spi_settings);

    // Initialize bme280
    struct bme280_t device;
    bme280Initialize(&device, spi_device);

    // Set bme280 settings
    struct bme280_settings_t settings = {
        .mode = bme280_mode_normal,
        .osr_temp = bme280_osr_temp_x16,
        .osr_pres = bme280_osr_pres_x16,
        .osr_hum = bme280_osr_hum_x16,
        .standby = bme280_standbytime_1000s,
        .filter = bme280_filter_coeff_16,
    };
    bme280SetDeviceSettings(&device, &settings);

    // Read Termperature, Pressure and Humidity
    struct bme280_data_t data;
    bme280ReadDeviceData(&device, &data);
    int32_t temperature =
        bme280CalculateTemperature(&device, &data);
    printf("temperature : %d\n", temperature);
    uint32_t pressure =
        bme280CalculatePressure(&device, &data);
    printf("pressure : %d\n", pressure);
    uint32_t humidity =
        bme280CalculateHumidity(&device, &data);
    printf("humidity : %d\n", humidity);

    // Finalize
    spiFinalize(spi_device);
    gpioTerminate();
    return 0;
}
