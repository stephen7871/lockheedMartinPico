#pragma once

#include <cstdint>
#include <string>
#include "sensor/bme280_driver.hpp"

std::string build_udp_packet(
    const std::string& protocol,
    const RawSample& sample,
    uint8_t chip_id
);