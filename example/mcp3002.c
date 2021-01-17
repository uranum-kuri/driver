#include <stdint.h>
#include <stdio.h>

#include "mcp3002.h"
#include "pigpio.h"
#include "spi.h"

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

    // Initialize mcp3002
    struct mcp3002_t device;
    mcp3002Initialize(&device, spi_device);

    // Set mcp3002 settings
    struct mcp3002_settings_t settings = {
        .base_voltage = 3300,
    };
    mcp3002SetDeviceSettings(&device, &settings);

    // Read Voltage
    struct mcp3002_data_t data;
    mcp3002ReadDeviceData(&device, &data);
    uint32_t voltage_differential_0 =
        mcp3002CalculateVoltage(&device, &data, mcp3002_channel_differential_0);
    printf("differential channel 0 voltage : %d\n", voltage_differential_0);
    uint32_t voltage_differential_1 =
        mcp3002CalculateVoltage(&device, &data, mcp3002_channel_differential_1);
    printf("differential channel 1 voltage : %d\n", voltage_differential_1);
    uint32_t voltage_single_0 =
        mcp3002CalculateVoltage(&device, &data, mcp3002_channel_single_0);
    printf("single channel 0 voltage : %d\n", voltage_single_0);
    uint32_t voltage_single_1 =
        mcp3002CalculateVoltage(&device, &data, mcp3002_channel_single_1);
    printf("single channel 1 voltage : %d\n", voltage_single_1);

    // Finalize
    spiFinalize(spi_device);
    gpioTerminate();
    return 0;
}
