#include "spi.h"
#include <stdint.h>
#include "pigpio.h"

spi_device_t spiInitialize(struct spi_settings_t* settings) {
    uint32_t spi_flag = 0;
    int8_t device;
    spi_flag = settings->mode_number | settings->active_high
                                           << settings->slave_number;
    device = spiOpen(settings->slave_number, settings->clock_speed, spi_flag);
    if (device >= 0) {
        return (spi_device_t)device;
    } else {
        return 0;
    }
}

void spiFinalize(spi_device_t device) {
    spiClose(device);
}

void spiTransfer(spi_device_t device, uint8_t* tx_buffer, uint8_t* rx_buffer,
                 uint16_t length) {
    if (rx_buffer == NULL) {
        spiWrite(device, (char*)tx_buffer, length);
    } else {
        spiXfer(device, (char*)tx_buffer, (char*)rx_buffer, length);
    }
}
