#pragma once

#include <cstdint>
#include "protocols/i2c_bus.hpp"

struct Bme280Sample {
    float temperature_c;
    float pressure_hpa;
    float humidity_percent;
};

class Bme280Driver {
public:
    explicit Bme280Driver(II2cBus& bus, uint8_t device_address = 0x76);

    bool init();
    bool read_sample(Bme280Sample& sample);

private:
    struct CalibrationData {
        uint16_t dig_T1{};
        int16_t  dig_T2{};
        int16_t  dig_T3{};

        uint16_t dig_P1{};
        int16_t  dig_P2{};
        int16_t  dig_P3{};
        int16_t  dig_P4{};
        int16_t  dig_P5{};
        int16_t  dig_P6{};
        int16_t  dig_P7{};
        int16_t  dig_P8{};
        int16_t  dig_P9{};

        uint8_t  dig_H1{};
        int16_t  dig_H2{};
        uint8_t  dig_H3{};
        int16_t  dig_H4{};
        int16_t  dig_H5{};
        int8_t   dig_H6{};
    };

    bool read_registers(uint8_t start_reg, uint8_t* buffer, uint8_t length);
    bool write_register(uint8_t reg, uint8_t value);
    bool read_calibration();

    float compensate_temperature(int32_t adc_T, int32_t& t_fine) const;
    float compensate_pressure(int32_t adc_P, int32_t t_fine) const;
    float compensate_humidity(int32_t adc_H, int32_t t_fine) const;

    II2cBus& bus_;
    uint8_t address_;
    CalibrationData cal_{};
};