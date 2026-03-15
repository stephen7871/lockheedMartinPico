#include "sensor/bme280_driver.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {
constexpr uint8_t REG_ID = 0xD0;
constexpr uint8_t REG_RESET = 0xE0;
constexpr uint8_t REG_CTRL_HUM = 0xF2;
constexpr uint8_t REG_STATUS = 0xF3;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_CONFIG = 0xF5;
constexpr uint8_t REG_DATA_START = 0xF7;

constexpr uint8_t BME280_CHIP_ID = 0x60;
} // namespace

BME280Driver::BME280Driver(const std::string& i2c_device, uint8_t address)
    : i2c_device_(i2c_device), address_(address) {}

BME280Driver::~BME280Driver() {
    closeDevice();
}

bool BME280Driver::openDevice() {
    fd_ = ::open(i2c_device_.c_str(), O_RDWR);
    if (fd_ < 0) {
        std::perror("open i2c");
        return false;
    }

    if (ioctl(fd_, I2C_SLAVE, address_) < 0) {
        std::perror("ioctl I2C_SLAVE");
        closeDevice();
        return false;
    }

    return true;
}

void BME280Driver::closeDevice() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool BME280Driver::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    ssize_t written = ::write(fd_, buffer, 2);
    return written == 2;
}

bool BME280Driver::readRegisters(uint8_t start_reg, uint8_t* buffer, size_t length) {
    if (::write(fd_, &start_reg, 1) != 1) {
        return false;
    }
    return ::read(fd_, buffer, length) == static_cast<ssize_t>(length);
}

uint8_t BME280Driver::readChipId() {
    uint8_t id = 0;
    if (!readRegisters(REG_ID, &id, 1)) {
        return 0;
    }
    return id;
}

bool BME280Driver::readCalibration() {
    uint8_t calib1[26] = {};
    uint8_t h1 = 0;
    uint8_t calib2[7] = {};

    if (!readRegisters(0x88, calib1, sizeof(calib1))) {
        return false;
    }
    if (!readRegisters(0xA1, &h1, 1)) {
        return false;
    }
    if (!readRegisters(0xE1, calib2, sizeof(calib2))) {
        return false;
    }

    calib_.dig_T1 = static_cast<uint16_t>(calib1[1] << 8 | calib1[0]);
    calib_.dig_T2 = static_cast<int16_t>(calib1[3] << 8 | calib1[2]);
    calib_.dig_T3 = static_cast<int16_t>(calib1[5] << 8 | calib1[4]);

    calib_.dig_P1 = static_cast<uint16_t>(calib1[7] << 8 | calib1[6]);
    calib_.dig_P2 = static_cast<int16_t>(calib1[9] << 8 | calib1[8]);
    calib_.dig_P3 = static_cast<int16_t>(calib1[11] << 8 | calib1[10]);
    calib_.dig_P4 = static_cast<int16_t>(calib1[13] << 8 | calib1[12]);
    calib_.dig_P5 = static_cast<int16_t>(calib1[15] << 8 | calib1[14]);
    calib_.dig_P6 = static_cast<int16_t>(calib1[17] << 8 | calib1[16]);
    calib_.dig_P7 = static_cast<int16_t>(calib1[19] << 8 | calib1[18]);
    calib_.dig_P8 = static_cast<int16_t>(calib1[21] << 8 | calib1[20]);
    calib_.dig_P9 = static_cast<int16_t>(calib1[23] << 8 | calib1[22]);

    calib_.dig_H1 = h1;
    calib_.dig_H2 = static_cast<int16_t>(calib2[1] << 8 | calib2[0]);
    calib_.dig_H3 = calib2[2];
    calib_.dig_H4 = static_cast<int16_t>((calib2[3] << 4) | (calib2[4] & 0x0F));
    calib_.dig_H5 = static_cast<int16_t>((calib2[5] << 4) | (calib2[4] >> 4));
    calib_.dig_H6 = static_cast<int8_t>(calib2[6]);

    return true;
}

bool BME280Driver::initialize() {
    if (!openDevice()) {
        return false;
    }

    uint8_t chip_id = readChipId();
    if (chip_id != BME280_CHIP_ID) {
        std::cerr << "Unexpected BME280 chip id: 0x" << std::hex
                  << static_cast<int>(chip_id) << std::dec << "\n";
        return false;
    }

    if (!writeRegister(REG_RESET, 0xB6)) {
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    if (!readCalibration()) {
        return false;
    }

    // humidity oversampling x1
    if (!writeRegister(REG_CTRL_HUM, 0x01)) {
        return false;
    }

    // temp x1, pressure x1, normal mode
    if (!writeRegister(REG_CTRL_MEAS, 0x27)) {
        return false;
    }

    // standby 1000 ms, filter off
    if (!writeRegister(REG_CONFIG, 0xA0)) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return true;
}

double BME280Driver::compensateTemperature(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - (static_cast<int32_t>(calib_.dig_T1) << 1))) *
                    static_cast<int32_t>(calib_.dig_T2)) >> 11;

    int32_t var2 = (((((adc_T >> 4) - static_cast<int32_t>(calib_.dig_T1)) *
                      ((adc_T >> 4) - static_cast<int32_t>(calib_.dig_T1))) >> 12) *
                    static_cast<int32_t>(calib_.dig_T3)) >> 14;

    t_fine_ = var1 + var2;
    int32_t T = (t_fine_ * 5 + 128) >> 8;
    return T / 100.0;
}

double BME280Driver::compensatePressure(int32_t adc_P) {
    int64_t var1 = static_cast<int64_t>(t_fine_) - 128000;
    int64_t var2 = var1 * var1 * static_cast<int64_t>(calib_.dig_P6);
    var2 = var2 + ((var1 * static_cast<int64_t>(calib_.dig_P5)) << 17);
    var2 = var2 + (static_cast<int64_t>(calib_.dig_P4) << 35);
    var1 = ((var1 * var1 * static_cast<int64_t>(calib_.dig_P3)) >> 8) +
           ((var1 * static_cast<int64_t>(calib_.dig_P2)) << 12);
    var1 = ((((static_cast<int64_t>(1) << 47) + var1)) *
            static_cast<int64_t>(calib_.dig_P1)) >> 33;

    if (var1 == 0) {
        return 0.0;
    }

    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(calib_.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (static_cast<int64_t>(calib_.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(calib_.dig_P7) << 4);

    // Pa -> hPa
    return (p / 256.0) / 100.0;
}

double BME280Driver::compensateHumidity(int32_t adc_H) {
    int32_t v_x1_u32r = t_fine_ - 76800;

    v_x1_u32r = (((((adc_H << 14) - (static_cast<int32_t>(calib_.dig_H4) << 20) -
                    (static_cast<int32_t>(calib_.dig_H5) * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * static_cast<int32_t>(calib_.dig_H6)) >> 10) *
                      (((v_x1_u32r * static_cast<int32_t>(calib_.dig_H3)) >> 11) + 32768)) >> 10) +
                    2097152) * static_cast<int32_t>(calib_.dig_H2) + 8192) >> 14));

    v_x1_u32r = v_x1_u32r -
                (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                  static_cast<int32_t>(calib_.dig_H1)) >> 4);

    if (v_x1_u32r < 0) {
        v_x1_u32r = 0;
    }
    if (v_x1_u32r > 419430400) {
        v_x1_u32r = 419430400;
    }

    return (v_x1_u32r >> 12) / 1024.0;
}

SensorReading BME280Driver::read() {
    uint8_t raw[8] = {};
    if (!readRegisters(REG_DATA_START, raw, sizeof(raw))) {
        throw std::runtime_error("Failed reading BME280 measurement registers");
    }

    int32_t adc_P = (static_cast<int32_t>(raw[0]) << 12) |
                    (static_cast<int32_t>(raw[1]) << 4) |
                    (static_cast<int32_t>(raw[2]) >> 4);

    int32_t adc_T = (static_cast<int32_t>(raw[3]) << 12) |
                    (static_cast<int32_t>(raw[4]) << 4) |
                    (static_cast<int32_t>(raw[5]) >> 4);

    int32_t adc_H = (static_cast<int32_t>(raw[6]) << 8) |
                    static_cast<int32_t>(raw[7]);

    SensorReading reading;
    reading.temperature_c = compensateTemperature(adc_T);
    reading.pressure_hpa = compensatePressure(adc_P);
    reading.humidity_percent = compensateHumidity(adc_H);
    return reading;
}
