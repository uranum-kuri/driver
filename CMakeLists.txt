cmake_minimum_required(VERSION 3.2)
project(driver C)

option(BUILD_SHARED_LIBS "shared library" ON)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3")

add_library(pigpio)
target_sources(pigpio PRIVATE lib/pigpio/pigpio.c
                              lib/pigpio/command.c
                              lib/pigpio/custom.cext)
target_include_directories(pigpio PUBLIC lib/pigpio)
target_link_libraries(pigpio PUBLIC pthread rt)

add_library(spi)
target_sources(spi PRIVATE src/spi.c)
target_include_directories(spi PRIVATE include)
target_link_libraries(spi PUBLIC pigpio)

add_library(i2c)
target_sources(i2c PRIVATE src/i2c.c)
target_include_directories(i2c PUBLIC include)
target_link_libraries(i2c PUBLIC pigpio)

add_library(bme280)
target_sources(bme280 PRIVATE src/bme280.c)
target_include_directories(bme280 PUBLIC include)
target_link_libraries(bme280 PUBLIC spi)

add_library(mcp3002)
target_sources(mcp3002 PRIVATE src/mcp3002.c)
target_include_directories(mcp3002 PUBLIC include)
target_link_libraries(mcp3002 PUBLIC spi)

add_library(tsl2561)
target_sources(tsl2561 PRIVATE src/tsl2561.c)
target_include_directories(tsl2561 PUBLIC include)
target_link_libraries(tsl2561 PUBLIC i2c)

add_executable(bme280_ex)
target_sources(bme280_ex PRIVATE example/bme280)
target_link_libraries(bme280_ex PRIVATE bme280)

add_executable(mcp3002_ex)
target_sources(mcp3002_ex PRIVATE example/mcp3002.c)
target_link_libraries(mcp3002_ex PRIVATE mcp3002)

add_executable(tsl2561_ex)
target_sources(tsl2561_ex PRIVATE example/tsl2561.c)
target_link_libraries(tsl2561_ex PRIVATE tsl2561)
