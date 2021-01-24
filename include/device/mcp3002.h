#ifndef __MCP3002_H__
#define __MCP3002_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "interface/spi.h"

enum mcp3002_channel_t {
    mcp3002_channel_differential_0 = 0x00,
    mcp3002_channel_differential_1 = 0x10,
    mcp3002_channel_single_0 = 0x20,
    mcp3002_channel_single_1 = 0x30,
};

#ifdef MCP3002_FLOAT_ENABLE

struct mcp3002_settings_t {
    double base_voltage;
};

#else

struct mcp3002_settings_t {
    uint32_t base_voltage;
};

#endif

struct mcp3002_data_t {
    uint16_t differential_0;
    uint16_t differential_1;
    uint16_t single_0;
    uint16_t single_1;
};

struct mcp3002_t {
    spi_device_t spi_device;
    struct mcp3002_settings_t settings;
};

void mcp3002Initialize(struct mcp3002_t* device, spi_device_t spi_device);

void mcp3002SetDeviceSettings(struct mcp3002_t* device,
                              struct mcp3002_settings_t* settings);

void mcp3002ReadDeviceData(struct mcp3002_t* device,
                           struct mcp3002_data_t* data);

#ifdef MCP3002_FLOAT_ENABLE

double mcp3002CalculateVoltage(struct mcp3002_t* device,
                               struct mcp3002_data_t* data,
                               enum mcp3002_channel_t channel);

#else

uint32_t mcp3002CalculateVoltage(struct mcp3002_t* device,
                                 struct mcp3002_data_t* data,
                                 enum mcp3002_channel_t channel);

#endif

#ifdef __cplusplus
}
#endif

#endif
