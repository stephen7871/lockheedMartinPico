#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "sensor/bme280_driver.hpp"

class PacketBuilder {
public:
    static std::string buildTelemetryJson(
        uint64_t sequence,
        const SensorReading& reading,
        bool spi_test_ran,
        bool spi_test_passed,
        const std::vector<uint8_t>& spi_tx,
        const std::vector<uint8_t>& spi_rx
    );

private:
    static std::string bytesToHexString(const std::vector<uint8_t>& bytes);
};
