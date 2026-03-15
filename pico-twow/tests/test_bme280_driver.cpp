#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "hw/i2c_bus.hpp"
#include "hw/spi_bus.hpp"
#include "sensor/bme280_driver.hpp"

class FakeI2CBus : public II2CBus {
public:
    uint8_t read_byte(uint8_t device_addr, uint8_t reg) override {
        last_device_addr = device_addr;
        if (!single_regs.count(reg)) {
            throw std::runtime_error("Missing fake single-byte register");
        }
        return single_regs[reg];
    }

    std::vector<uint8_t> read_bytes(uint8_t device_addr, uint8_t start_reg, std::size_t count) override {
        last_device_addr = device_addr;
        last_start_reg = start_reg;
        if (!block_regs.count(start_reg)) {
            throw std::runtime_error("Missing fake block register");
        }
        auto data = block_regs[start_reg];
        if (data.size() != count) {
            throw std::runtime_error("Unexpected block length");
        }
        return data;
    }

    void write_byte(uint8_t device_addr, uint8_t reg, uint8_t value) override {
        last_device_addr = device_addr;
        writes.push_back({reg, value});
    }

    struct WriteOp {
        uint8_t reg;
        uint8_t value;
    };

    uint8_t last_device_addr{};
    uint8_t last_start_reg{};
    std::map<uint8_t, uint8_t> single_regs;
    std::map<uint8_t, std::vector<uint8_t>> block_regs;
    std::vector<WriteOp> writes;
};

class FakeSpiBus : public ISpiBus {
public:
    std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx) override {
        transfers.push_back(tx);

        // chip-id read
        if (tx.size() == 2 && tx[0] == 0xD0u + 0x80u) {
            return {tx[0], 0x60};
        }

        // raw data read
        if (tx.size() == 9 && tx[0] == 0xF7u + 0x80u) {
            return {
                tx[0],
                0x64, 0x00, 0x00,   // pressure
                0x80, 0x00, 0x00,   // temperature
                0x12, 0x34          // humidity
            };
        }

        // register writes just echo
        return tx;
    }

    std::vector<std::vector<uint8_t>> transfers;
};

TEST(Bme280I2cDriverTest, InitializesAndReadsRawData) {
    FakeI2CBus bus;
    bus.single_regs[0xD0] = 0x60;
    bus.block_regs[0xF7] = {
        0x64, 0x00, 0x00,
        0x80, 0x00, 0x00,
        0x12, 0x34
    };

    Bme280I2cDriver driver(bus, 0x76);

    EXPECT_TRUE(driver.initialize());
    ASSERT_EQ(bus.writes.size(), 3u);
    EXPECT_EQ(bus.writes[0].reg, 0xF2);
    EXPECT_EQ(bus.writes[1].reg, 0xF4);
    EXPECT_EQ(bus.writes[2].reg, 0xF5);

    const auto sample = driver.read_raw_sample();
    EXPECT_EQ(sample.pressure_raw, 0x64000u);
    EXPECT_EQ(sample.temperature_raw, 0x80000u);
    EXPECT_EQ(sample.humidity_raw, 0x1234u);
}

TEST(Bme280SpiDriverTest, InitializesAndReadsRawData) {
    FakeSpiBus bus;
    Bme280SpiDriver driver(bus);

    EXPECT_TRUE(driver.initialize());

    const auto sample = driver.read_raw_sample();
    EXPECT_EQ(sample.pressure_raw, 0x64000u);
    EXPECT_EQ(sample.temperature_raw, 0x80000u);
    EXPECT_EQ(sample.humidity_raw, 0x1234u);

    ASSERT_GE(bus.transfers.size(), 4u);
}