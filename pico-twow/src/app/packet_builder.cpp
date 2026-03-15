#include "app/packet_builder.hpp"

#include <iomanip>
#include <sstream>

std::string PacketBuilder::bytesToHexString(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < bytes.size(); ++i) {
        oss << "\"0x"
            << std::uppercase
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(bytes[i])
            << "\"";
        if (i + 1 < bytes.size()) {
            oss << ",";
        }
    }
    oss << "]";
    return oss.str();
}

std::string PacketBuilder::buildTelemetryJson(
    uint64_t sequence,
    const SensorReading& reading,
    bool spi_test_ran,
    bool spi_test_passed,
    const std::vector<uint8_t>& spi_tx,
    const std::vector<uint8_t>& spi_rx
) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{";
    oss << "\"sequence\":" << sequence << ",";
    oss << "\"protocols\":{";
    oss << "\"hardware\":[\"I2C\",\"SPI\"],";
    oss << "\"network\":[\"IP\",\"UDP\"]";
    oss << "},";
    oss << "\"sensor\":{";
    oss << "\"temperature_c\":" << reading.temperature_c << ",";
    oss << "\"humidity_percent\":" << reading.humidity_percent << ",";
    oss << "\"pressure_hpa\":" << reading.pressure_hpa;
    oss << "},";
    oss << "\"spi_demo\":{";
    oss << "\"ran\":" << (spi_test_ran ? "true" : "false") << ",";
    oss << "\"passed\":" << (spi_test_passed ? "true" : "false") << ",";
    oss << "\"tx\":" << bytesToHexString(spi_tx) << ",";
    oss << "\"rx\":" << bytesToHexString(spi_rx);
    oss << "}";
    oss << "}";
    return oss.str();
}
