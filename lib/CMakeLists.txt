add_library(pigpio)
target_sources(pigpio PRIVATE pigpio/pigpio.c
                              pigpio/command.c
                              pigpio/custom.cext)
target_include_directories(pigpio PUBLIC pigpio)
target_link_libraries(pigpio PUBLIC pthread rt)
