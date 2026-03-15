#include <gtest/gtest.h>
#include <string>

#include "app/packet_builder.hpp"
#include "sensor/bme280_driver.hpp"

TEST(PacketBuilderTest, BuildsExpectedJsonFields) {
    RawSample sample{};
    sample.pressure_raw = 100000;
    sample.temperature_raw = 200000;
    sample.humidity_raw = 30000;

    const std::string packet = build_udp_packet("i2c", sample, 0x60);

    EXPECT_NE(packet.find("\"sensor\":\"bme280\""), std::string::npos);
    EXPECT_NE(packet.find("\"transport\":\"udp\""), std::string::npos);
    EXPECT_NE(packet.find("\"hw_protocol\":\"i2c\""), std::string::npos);
    EXPECT_NE(packet.find("\"chip_id\":96"), std::string::npos);
    EXPECT_NE(packet.find("\"pressure_raw\":100000"), std::string::npos);
    EXPECT_NE(packet.find("\"temperature_raw\":200000"), std::string::npos);
    EXPECT_NE(packet.find("\"humidity_raw\":30000"), std::string::npos);
}