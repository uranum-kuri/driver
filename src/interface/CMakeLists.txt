add_library(spi)
target_sources(spi PRIVATE spi.c)
target_include_directories(spi PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(spi PUBLIC pigpio)

add_library(i2c)
target_sources(i2c PRIVATE i2c.c)
target_include_directories(i2c PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(i2c PUBLIC pigpio)
