add_executable(bme280_ex)
target_sources(bme280_ex PRIVATE bme280)
target_link_libraries(bme280_ex PRIVATE bme280)

add_executable(mcp3002_ex)
target_sources(mcp3002_ex PRIVATE mcp3002.c)
target_link_libraries(mcp3002_ex PRIVATE mcp3002)

add_executable(tsl2561_ex)
target_sources(tsl2561_ex PRIVATE tsl2561.c)
target_link_libraries(tsl2561_ex PRIVATE tsl2561)
