#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum spi_active_high_t {
    spi_active_high_disable = 0x00,
    spi_active_high_enable = 0x04,
};

struct spi_settings_t {
    uint8_t slave_number;
    uint32_t clock_speed;
    uint8_t mode_number;
    enum spi_active_high_t active_high;
};

typedef uint8_t spi_device_t;

spi_device_t spiInitialize(struct spi_settings_t* settings);
void spiFinalize(spi_device_t device);

void spiTransfer(spi_device_t device, uint8_t* tx_buffer, uint8_t* rx_buffer,
                 uint16_t length);

#ifdef __cplusplus
}
#endif

#endif
