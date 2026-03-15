#pragma once

#include <cstdint>
#include <string>
#include "hw/i2c_bus.hpp"
#include "hw/spi_bus.hpp"

struct RawSample {
    uint32_t pressure_raw{};
    uint32_t temperature_raw{};
    uint16_t humidity_raw{};
};

class Bme280I2cDriver {
public:
    Bme280I2cDriver(II2CBus& bus, uint8_t device_addr);

    bool initialize();
    uint8_t read_chip_id();
    RawSample read_raw_sample();

private:
    II2CBus& bus_;
    uint8_t addr_;
};

class Bme280SpiDriver {
public:
    explicit Bme280SpiDriver(ISpiBus& bus);

    bool initialize();
    uint8_t read_chip_id();
    RawSample read_raw_sample();

private:
    ISpiBus& bus_;
};