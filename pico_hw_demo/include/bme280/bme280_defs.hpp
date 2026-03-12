#pragma once

#include <array>
#include <cstdint>

namespace bme280 {

constexpr std::uint8_t CHIP_ID = 0x60;

constexpr std::uint8_t REG_ID = 0xD0;
constexpr std::uint8_t REG_RESET = 0xE0;
constexpr std::uint8_t REG_CTRL_HUM = 0xF2;
constexpr std::uint8_t REG_STATUS = 0xF3;
constexpr std::uint8_t REG_CTRL_MEAS = 0xF4;
constexpr std::uint8_t REG_CONFIG = 0xF5;
constexpr std::uint8_t REG_PRESS_MSB = 0xF7;

constexpr std::uint8_t REG_CALIB_00 = 0x88;
constexpr std::uint8_t REG_CALIB_26 = 0xE1;

constexpr std::uint8_t SOFT_RESET_CMD = 0xB6;

struct CalibrationData {
    std::uint16_t dig_T1 {};
    std::int16_t dig_T2 {};
    std::int16_t dig_T3 {};

    std::uint16_t dig_P1 {};
    std::int16_t dig_P2 {};
    std::int16_t dig_P3 {};
    std::int16_t dig_P4 {};
    std::int16_t dig_P5 {};
    std::int16_t dig_P6 {};
    std::int16_t dig_P7 {};
    std::int16_t dig_P8 {};
    std::int16_t dig_P9 {};

    std::uint8_t dig_H1 {};
    std::int16_t dig_H2 {};
    std::uint8_t dig_H3 {};
    std::int16_t dig_H4 {};
    std::int16_t dig_H5 {};
    std::int8_t dig_H6 {};
};

inline std::int16_t u16_to_s16(std::uint16_t value) {
    return static_cast<std::int16_t>(value);
}

} // namespace bme280
