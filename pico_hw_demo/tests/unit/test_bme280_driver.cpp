#include <cstdint>
#include <map>
#include <vector>

#include "gtest/gtest.h"
#include "bme280/bme280_driver.hpp"
#include "protocols/i2c_bus.hpp"

class FakeI2cBus : public II2cBus {
public:
    bool write(uint8_t, const uint8_t* data, std::size_t length) override {
        if (length >= 2) {
            registers_[data[0]] = data[1];
        }
        writes_.push_back(std::vector<uint8_t>(data, data + length));
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

    const std::vector<std::vector<uint8_t>>& writes() const {
        return writes_;
    }

private:
    std::map<uint8_t, uint8_t> registers_;
    std::vector<std::vector<uint8_t>> writes_;
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

TEST(Bme280DriverUnitTest, InitSucceedsWithValidChipIdAndCalibration) {
    FakeI2cBus bus;
    load_calibration(bus);

    Bme280Driver driver(bus, 0x76);
    EXPECT_TRUE(driver.init());

    ASSERT_GE(bus.writes().size(), 3u);
    EXPECT_EQ(bus.writes()[0][0], 0xE0);
    EXPECT_EQ(bus.writes()[1][0], 0xF2);
    EXPECT_EQ(bus.writes()[2][0], 0xF4);
}

TEST(Bme280DriverUnitTest, InitFailsWithWrongChipId) {
    FakeI2cBus bus;
    bus.set_reg(0xD0, 0x58);

    Bme280Driver driver(bus, 0x76);
    EXPECT_FALSE(driver.init());
}