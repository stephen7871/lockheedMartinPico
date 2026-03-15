#include "app/packet_builder.hpp"

#include <sstream>

std::string build_udp_packet(
    const std::string& protocol,
    const RawSample& sample,
    uint8_t chip_id
) {
    std::ostringstream oss;
    oss << "{"
        << "\"sensor\":\"bme280\","
        << "\"transport\":\"udp\","
        << "\"hw_protocol\":\"" << protocol << "\","
        << "\"chip_id\":" << static_cast<unsigned>(chip_id) << ","
        << "\"pressure_raw\":" << sample.pressure_raw << ","
        << "\"temperature_raw\":" << sample.temperature_raw << ","
        << "\"humidity_raw\":" << sample.humidity_raw
        << "}";

    return oss.str();
}