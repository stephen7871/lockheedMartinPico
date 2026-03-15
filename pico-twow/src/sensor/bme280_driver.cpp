#include "sensor/bme280_driver.hpp"

#include <stdexcept>
#include <vector>

namespace {
constexpr uint8_t REG_CHIP_ID   = 0xD0;
constexpr uint8_t REG_CTRL_HUM  = 0xF2;
constexpr uint8_t REG_STATUS    = 0xF3;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_CONFIG    = 0xF5;
constexpr uint8_t REG_DATA      = 0xF7;
constexpr uint8_t CHIP_ID_BME280 = 0x60;

RawSample parse_raw(const std::vector<uint8_t>& data) {
    if (data.size() != 8) {
        throw std::runtime_error("Expected 8 bytes from sensor data register block");
    }

    RawSample sample{};
    sample.pressure_raw =
        (static_cast<uint32_t>(data[0]) << 12) |
        (static_cast<uint32_t>(data[1]) << 4)  |
        (static_cast<uint32_t>(data[2]) >> 4);

    sample.temperature_raw =
        (static_cast<uint32_t>(data[3]) << 12) |
        (static_cast<uint32_t>(data[4]) << 4)  |
        (static_cast<uint32_t>(data[5]) >> 4);

    sample.humidity_raw =
        (static_cast<uint16_t>(data[6]) << 8) |
        static_cast<uint16_t>(data[7]);

    return sample;
}
}

Bme280I2cDriver::Bme280I2cDriver(II2CBus& bus, uint8_t device_addr)
    : bus_(bus), addr_(device_addr) {}

bool Bme280I2cDriver::initialize() {
    if (read_chip_id() != CHIP_ID_BME280) {
        return false;
    }

    // humidity oversampling x1
    bus_.write_byte(addr_, REG_CTRL_HUM, 0x01);

    // temp oversampling x1, pressure oversampling x1, normal mode
    bus_.write_byte(addr_, REG_CTRL_MEAS, 0x27);

    // standby 1000 ms, filter off
    bus_.write_byte(addr_, REG_CONFIG, 0xA0);

    return true;
}

uint8_t Bme280I2cDriver::read_chip_id() {
    return bus_.read_byte(addr_, REG_CHIP_ID);
}

RawSample Bme280I2cDriver::read_raw_sample() {
    return parse_raw(bus_.read_bytes(addr_, REG_DATA, 8));
}

Bme280SpiDriver::Bme280SpiDriver(ISpiBus& bus)
    : bus_(bus) {}

bool Bme280SpiDriver::initialize() {
    if (read_chip_id() != CHIP_ID_BME280) {
        return false;
    }

    // SPI write: bit7 = 0
    bus_.transfer({REG_CTRL_HUM & 0x7F, 0x01});
    bus_.transfer({REG_CTRL_MEAS & 0x7F, 0x27});
    bus_.transfer({REG_CONFIG & 0x7F, 0xA0});

    return true;
}

uint8_t Bme280SpiDriver::read_chip_id() {
    // SPI read: bit7 = 1
    const auto rx = bus_.transfer({static_cast<uint8_t>(REG_CHIP_ID | 0x80), 0x00});
    if (rx.size() < 2) {
        throw std::runtime_error("SPI chip-id read returned too few bytes");
    }
    return rx[1];
}

RawSample Bme280SpiDriver::read_raw_sample() {
    std::vector<uint8_t> tx(9, 0x00);
    tx[0] = static_cast<uint8_t>(REG_DATA | 0x80);

    const auto rx = bus_.transfer(tx);
    if (rx.size() != 9) {
        throw std::runtime_error("SPI data read returned wrong number of bytes");
    }

    std::vector<uint8_t> data(rx.begin() + 1, rx.end());
    return parse_raw(data);
}