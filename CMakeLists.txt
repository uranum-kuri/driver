cmake_minimum_required(VERSION 3.2)
project(driver C)

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

add_library(i2c)
target_sources(i2c PRIVATE src/i2c.c)
target_include_directories(i2c PUBLIC include)
target_link_libraries(i2c PUBLIC pigpio)
