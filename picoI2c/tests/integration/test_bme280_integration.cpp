#include <cstdint>
#include <map>

#include "gtest/gtest.h"
#include "bme280/bme280_driver.hpp"
#include "protocols/i2c_bus.hpp"

class FakeI2cBus : public II2cBus {
public:
    bool write(uint8_t, const uint8_t* data, std::size_t length) override {
        if (length >= 2) {
            registers_[data[0]] = data[1];
        }
        return true;
    }

    bool write_read(uint8_t,
                    const uint8_t* write_data,
                    std::size_t write_length,
                    uint8_t* read_data,
                    std::size_t read_length) override {
        if (write_length != 1) {
            return false;
        }

        uint8_t reg = write_data[0];
        for (std::size_t i = 0; i < read_length; ++i) {
            read_data[i] = registers_[static_cast<uint8_t>(reg + i)];
        }
        return true;
    }

    void set_reg(uint8_t reg, uint8_t value) {
        registers_[reg] = value;
    }

private:
    std::map<uint8_t, uint8_t> registers_;
};

static void load_calibration(FakeI2cBus& bus) {
    bus.set_reg(0xD0, 0x60);

    const uint8_t calib1[] = {
        0x70, 0x6B, 0x43, 0x67, 0x18, 0xFC,
        0x7D, 0x8E, 0x43, 0xD6, 0xD0, 0x0B,
        0x27, 0x0B, 0x8C, 0x00, 0xF9, 0xFF,
        0x8C, 0x3C, 0xF8, 0xC6, 0x70, 0x17,
        0x4B, 0x6A
    };

    for (int i = 0; i < 26; ++i) {
        bus.set_reg(static_cast<uint8_t>(0x88 + i), calib1[i]);
    }

    const uint8_t calib2[] = {0x6A, 0x01, 0x00, 0x13, 0x2D, 0x03, 0x1E};
    for (int i = 0; i < 7; ++i) {
        bus.set_reg(static_cast<uint8_t>(0xE1 + i), calib2[i]);
    }
}

static void load_raw_sample(FakeI2cBus& bus) {
    // Example raw registers F7..FE
    bus.set_reg(0xF7, 0x65);
    bus.set_reg(0xF8, 0x5A);
    bus.set_reg(0xF9, 0xC0);

    bus.set_reg(0xFA, 0x7E);
    bus.set_reg(0xFB, 0xED);
    bus.set_reg(0xFC, 0x00);

    bus.set_reg(0xFD, 0x66);
    bus.set_reg(0xFE, 0x80);
}

TEST(Bme280IntegrationTest, InitAndReadSampleProduceSaneValues) {
    FakeI2cBus bus;
    load_calibration(bus);
    load_raw_sample(bus);

    Bme280Driver driver(bus, 0x76);
    ASSERT_TRUE(driver.init());

    Bme280Sample sample{};
    ASSERT_TRUE(driver.read_sample(sample));

    EXPECT_GT(sample.temperature_c, -40.0f);
    EXPECT_LT(sample.temperature_c, 85.0f);

    EXPECT_GT(sample.pressure_hpa, 300.0f);
    EXPECT_LT(sample.pressure_hpa, 1100.0f);

    EXPECT_GE(sample.humidity_percent, 0.0f);
    EXPECT_LE(sample.humidity_percent, 100.0f);
}