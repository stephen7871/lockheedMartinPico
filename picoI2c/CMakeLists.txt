cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350)

option(TARGET_PICO "Build Pico firmware" ON)
option(BUILD_HOST_TESTS "Build host GoogleTest tests" OFF)

if(TARGET_PICO)
    if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH is not set. Export it before running CMake.")
    endif()

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    endif()

    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

project(pico_hw_demo C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(TARGET_PICO)
    pico_sdk_init()

    add_executable(sensor_demo
        src/main.cpp
        src/bme280_driver.cpp
    )

    target_include_directories(sensor_demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(sensor_demo
        pico_stdlib
        hardware_i2c
        hardware_gpio
    )

    pico_enable_stdio_usb(sensor_demo 1)
    pico_enable_stdio_uart(sensor_demo 0)
    pico_add_extra_outputs(sensor_demo)
endif()

if(BUILD_HOST_TESTS AND NOT TARGET_PICO)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    add_library(bme280_host_lib
        src/bme280_driver.cpp
    )

    target_include_directories(bme280_host_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    add_executable(test_bme280_driver
        tests/unit/test_bme280_driver.cpp
    )
    target_link_libraries(test_bme280_driver PRIVATE
        bme280_host_lib
        GTest::gtest_main
    )

    add_executable(test_bme280_integration
        tests/integration/test_bme280_integration.cpp
    )
    target_link_libraries(test_bme280_integration PRIVATE
        bme280_host_lib
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(test_bme280_driver)
    gtest_discover_tests(test_bme280_integration)
endif()