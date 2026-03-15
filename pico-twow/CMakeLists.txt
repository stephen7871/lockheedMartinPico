cmake_minimum_required(VERSION 3.16)
project(pi_zero_telemetry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)

add_executable(pi_zero_telemetry
    src/main.cpp
    src/app/packet_builder.cpp
    src/hardware/spi_demo.cpp
    src/network/udp_sender.cpp
    src/sensor/bme280_driver.cpp
)
