#include "bme280/bme280_driver.hpp"

#include <cmath>
#include <cstring>

namespace {
constexpr uint8_t REG_ID        = 0xD0;
constexpr uint8_t REG_RESET     = 0xE0;
constexpr uint8_t REG_CTRL_HUM  = 0xF2;
constexpr uint8_t REG_STATUS    = 0xF3;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_CONFIG    = 0xF5;
constexpr uint8_t REG_DATA      = 0xF7;

constexpr uint8_t CHIP_ID       = 0x60;
constexpr uint8_t RESET_CMD     = 0xB6;
}

Bme280Driver::Bme280Driver(II2cBus& bus, uint8_t device_address)
    : bus_(bus), address_(device_address) {}

bool Bme280Driver::write_register(uint8_t reg, uint8_t value) {
    uint8_t payload[2] = {reg, value};
    return bus_.write(address_, payload, 2);
}

bool Bme280Driver::read_registers(uint8_t start_reg, uint8_t* buffer, uint8_t length) {
    return bus_.write_read(address_, &start_reg, 1, buffer, length);
}

bool Bme280Driver::read_calibration() {
    uint8_t calib1[26] = {};
    uint8_t calib2[7] = {};

    if (!read_registers(0x88, calib1, sizeof(calib1))) {
        return false;
    }
    if (!read_registers(0xE1, calib2, sizeof(calib2))) {
        return false;
    }

    cal_.dig_T1 = static_cast<uint16_t>(calib1[1] << 8 | calib1[0]);
    cal_.dig_T2 = static_cast<int16_t>(calib1[3] << 8 | calib1[2]);
    cal_.dig_T3 = static_cast<int16_t>(calib1[5] << 8 | calib1[4]);

    cal_.dig_P1 = static_cast<uint16_t>(calib1[7] << 8 | calib1[6]);
    cal_.dig_P2 = static_cast<int16_t>(calib1[9] << 8 | calib1[8]);
    cal_.dig_P3 = static_cast<int16_t>(calib1[11] << 8 | calib1[10]);
    cal_.dig_P4 = static_cast<int16_t>(calib1[13] << 8 | calib1[12]);
    cal_.dig_P5 = static_cast<int16_t>(calib1[15] << 8 | calib1[14]);
    cal_.dig_P6 = static_cast<int16_t>(calib1[17] << 8 | calib1[16]);
    cal_.dig_P7 = static_cast<int16_t>(calib1[19] << 8 | calib1[18]);
    cal_.dig_P8 = static_cast<int16_t>(calib1[21] << 8 | calib1[20]);
    cal_.dig_P9 = static_cast<int16_t>(calib1[23] << 8 | calib1[22]);

    cal_.dig_H1 = calib1[25];
    cal_.dig_H2 = static_cast<int16_t>(calib2[1] << 8 | calib2[0]);
    cal_.dig_H3 = calib2[2];
    cal_.dig_H4 = static_cast<int16_t>((calib2[3] << 4) | (calib2[4] & 0x0F));
    cal_.dig_H5 = static_cast<int16_t>((calib2[5] << 4) | (calib2[4] >> 4));
    cal_.dig_H6 = static_cast<int8_t>(calib2[6]);

    return true;
}

bool Bme280Driver::init() {
    uint8_t id = 0;
    if (!read_registers(REG_ID, &id, 1)) {
        return false;
    }
    if (id != CHIP_ID) {
        return false;
    }

    if (!write_register(REG_RESET, RESET_CMD)) {
        return false;
    }

    if (!read_calibration()) {
        return false;
    }

    if (!write_register(REG_CTRL_HUM, 0x01)) {
        return false;
    }

    if (!write_register(REG_CTRL_MEAS, 0x27)) {
        return false;
    }

    if (!write_register(REG_CONFIG, 0xA0)) {
        return false;
    }

    return true;
}

float Bme280Driver::compensate_temperature(int32_t adc_T, int32_t& t_fine) const {
    const int32_t var1 = ((((adc_T >> 3) - (static_cast<int32_t>(cal_.dig_T1) << 1))) *
                           static_cast<int32_t>(cal_.dig_T2)) >> 11;

    const int32_t var2 = (((((adc_T >> 4) - static_cast<int32_t>(cal_.dig_T1)) *
                             ((adc_T >> 4) - static_cast<int32_t>(cal_.dig_T1))) >> 12) *
                           static_cast<int32_t>(cal_.dig_T3)) >> 14;

    t_fine = var1 + var2;
    const float temperature = (t_fine * 5 + 128) / 256.0f;
    return temperature / 100.0f;
}

float Bme280Driver::compensate_pressure(int32_t adc_P, int32_t t_fine) const {
    double var1 = (static_cast<double>(t_fine) / 2.0) - 64000.0;
    double var2 = var1 * var1 * static_cast<double>(cal_.dig_P6) / 32768.0;
    var2 = var2 + var1 * static_cast<double>(cal_.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (static_cast<double>(cal_.dig_P4) * 65536.0);
    var1 = (static_cast<double>(cal_.dig_P3) * var1 * var1 / 524288.0 +
            static_cast<double>(cal_.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * static_cast<double>(cal_.dig_P1);

    if (var1 == 0.0) {
        return 0.0f;
    }

    double pressure = 1048576.0 - static_cast<double>(adc_P);
    pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = static_cast<double>(cal_.dig_P9) * pressure * pressure / 2147483648.0;
    var2 = pressure * static_cast<double>(cal_.dig_P8) / 32768.0;
    pressure = pressure + (var1 + var2 + static_cast<double>(cal_.dig_P7)) / 16.0;

    return static_cast<float>(pressure / 100.0);
}

float Bme280Driver::compensate_humidity(int32_t adc_H, int32_t t_fine) const {
    double h = static_cast<double>(t_fine) - 76800.0;
    h = (static_cast<double>(adc_H) -
        (static_cast<double>(cal_.dig_H4) * 64.0 +
         static_cast<double>(cal_.dig_H5) / 16384.0 * h)) *
        (static_cast<double>(cal_.dig_H2) / 65536.0 *
        (1.0 + static_cast<double>(cal_.dig_H6) / 67108864.0 * h *
        (1.0 + static_cast<double>(cal_.dig_H3) / 67108864.0 * h)));

    h = h * (1.0 - static_cast<double>(cal_.dig_H1) * h / 524288.0);

    if (h > 100.0) h = 100.0;
    if (h < 0.0) h = 0.0;

    return static_cast<float>(h);
}

bool Bme280Driver::read_sample(Bme280Sample& sample) {
    uint8_t data[8] = {};
    if (!read_registers(REG_DATA, data, sizeof(data))) {
        return false;
    }

    const int32_t adc_P = (static_cast<int32_t>(data[0]) << 12) |
                          (static_cast<int32_t>(data[1]) << 4) |
                          (static_cast<int32_t>(data[2]) >> 4);

    const int32_t adc_T = (static_cast<int32_t>(data[3]) << 12) |
                          (static_cast<int32_t>(data[4]) << 4) |
                          (static_cast<int32_t>(data[5]) >> 4);

    const int32_t adc_H = (static_cast<int32_t>(data[6]) << 8) |
                           static_cast<int32_t>(data[7]);

    int32_t t_fine = 0;
    sample.temperature_c = compensate_temperature(adc_T, t_fine);
    sample.pressure_hpa = compensate_pressure(adc_P, t_fine);
    sample.humidity_percent = compensate_humidity(adc_H, t_fine);

    return std::isfinite(sample.temperature_c) &&
           std::isfinite(sample.pressure_hpa) &&
           std::isfinite(sample.humidity_percent);
}