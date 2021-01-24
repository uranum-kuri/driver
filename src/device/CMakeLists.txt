add_library(bme280)
target_sources(bme280 PRIVATE bme280.c)
target_include_directories(bme280 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(bme280 PUBLIC spi)

add_library(mcp3002)
target_sources(mcp3002 PRIVATE mcp3002.c)
target_include_directories(mcp3002 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(mcp3002 PUBLIC spi)

add_library(tsl2561)
target_sources(tsl2561 PRIVATE tsl2561.c)
target_include_directories(tsl2561 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tsl2561 PUBLIC i2c)
