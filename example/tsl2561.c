#include <stdio.h>
#include <stdint.h>

#include "pigpio.h"
#include "i2c.h"
#include "tsl2561.h"

int main(void) {
    // initialize pigpio
    gpioInitialise();

    // initialize i2c device
    struct i2c_settings_t i2c_settings = {
        .bus_number = 1,
        .address = tsl2561_addr_float,
    };
    i2c_device_t i2c_device = i2cInitialize(&i2c_settings);

    // initialize tsl2561
    struct tsl2561_t device;
    tsl2561Initialize(&device, i2c_device);

    // set tsl2561 settings
    struct tsl2561_settings_t settings = {
        .gain = tsl2561_gain_16x,
        .integral = tsl2561_integral_402ms,
    };
    tsl2561SetDeviceSettings(&device, &settings);

    // read illuminance
    struct tsl2561_data_t data;
    tsl2561ReadDeviceData(&device, &data);
    uint32_t illuminance = tsl2561CalculateIlluminance(&device, &data);
    printf("illuminance : %d\n", illuminance);

    // finalize
    i2cFinalize(i2c_device);
    gpioTerminate();
    return 0;
}
