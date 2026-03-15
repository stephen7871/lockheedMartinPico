cmake_minimum_required(VERSION 3.16)
project(protocol_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_library(core
    src/app/packet_builder.cpp
    src/net/udp_client.cpp
    src/sensor/bme280_driver.cpp
)

target_include_directories(core PUBLIC include)

if(UNIX AND NOT APPLE)
    add_executable(protocol_demo
        src/main.cpp
        src/hw/linux_i2c_bus.cpp
        src/hw/linux_spi_bus.cpp
    )
    target_include_directories(protocol_demo PRIVATE include)
    target_link_libraries(protocol_demo PRIVATE core)
endif()

enable_testing()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    tests/test_bme280_driver.cpp
    tests/test_packet_builder.cpp
)

target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)