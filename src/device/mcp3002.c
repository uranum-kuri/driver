#include <stdint.h>

#include "interface/spi.h"
#include "device/mcp3002.h"

#include <unistd.h>
void delay(uint16_t ms) {
    usleep(ms * 1000);
}

static uint16_t mcp3002GetReg(uint8_t device, enum mcp3002_channel_t channel);

void mcp3002Initialize(struct mcp3002_t* device, spi_device_t spi_device) {
    device->spi_device = spi_device;
    struct mcp3002_settings_t settings = {
        .base_voltage = 0,
    };
    mcp3002SetDeviceSettings(device, &settings);
}

void mcp3002SetDeviceSettings(struct mcp3002_t* device,
                              struct mcp3002_settings_t* settings) {
    device->settings.base_voltage = settings->base_voltage;
}

void mcp3002ReadDeviceData(struct mcp3002_t* device,
                           struct mcp3002_data_t* data) {
    delay(100);
    data->differential_0 =
        mcp3002GetReg(device->spi_device, mcp3002_channel_differential_0);
    data->differential_1 =
        mcp3002GetReg(device->spi_device, mcp3002_channel_differential_1);
    data->single_0 =
        mcp3002GetReg(device->spi_device, mcp3002_channel_single_0);
    data->single_1 =
        mcp3002GetReg(device->spi_device, mcp3002_channel_single_1);
}

#ifdef MCP3002_FLOAT_ENABLE

double mcp3002CalculateVoltage(struct mcp3002_t* device,
                               struct mcp3002_data_t* data,
                               enum mcp3002_channel_t channel) {
    uint16_t adc = 0;
    switch (channel) {
    case mcp3002_channel_differential_0:
        adc = data->differential_0;
        break;
    case mcp3002_channel_differential_1:
        adc = data->differential_1;
        break;
    case mcp3002_channel_single_0:
        adc = data->single_0;
        break;
    case mcp3002_channel_single_1:
        adc = data->single_1;
        break;
    }
    double voltage;
    voltage = adc * device->settings.base_voltage / 1024.0;
    return voltage;
}

#else

uint32_t mcp3002CalculateVoltage(struct mcp3002_t* device,
                                 struct mcp3002_data_t* data,
                                 enum mcp3002_channel_t channel) {
    uint16_t adc = 0;
    switch (channel) {
    case mcp3002_channel_differential_0:
        adc = data->differential_0;
        break;
    case mcp3002_channel_differential_1:
        adc = data->differential_1;
        break;
    case mcp3002_channel_single_0:
        adc = data->single_0;
        break;
    case mcp3002_channel_single_1:
        adc = data->single_1;
        break;
    }
    uint32_t voltage;
    voltage = adc * device->settings.base_voltage / 1024;
    return voltage;
}

#endif

static uint16_t mcp3002GetReg(uint8_t device, enum mcp3002_channel_t channel) {
    uint8_t tx_buf[2];
    uint8_t rx_buf[2];
    tx_buf[0] = 0x48 | channel;
    spiTransfer(device, tx_buf, rx_buf, 2);
    uint16_t data = 0;
    data = (rx_buf[0] | 0x03) << 8 | rx_buf[1];
    return data;
}
