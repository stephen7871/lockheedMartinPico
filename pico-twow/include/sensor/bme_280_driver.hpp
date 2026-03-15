#pragma once

#include <cstdint>
#include <string>

struct SensorReading {
    double temperature_c = 0.0;
    double humidity_percent = 0.0;
    double pressure_hpa = 0.0;
};

class BME280Driver {
public:
    BME280Driver(const std::string& i2c_device, uint8_t address);
    ~BME280Driver();

    bool initialize();
    SensorReading read();

private:
    struct CalibrationData {
        uint16_t dig_T1 = 0;
        int16_t  dig_T2 = 0;
        int16_t  dig_T3 = 0;

        uint16_t dig_P1 = 0;
        int16_t  dig_P2 = 0;
        int16_t  dig_P3 = 0;
        int16_t  dig_P4 = 0;
        int16_t  dig_P5 = 0;
        int16_t  dig_P6 = 0;
        int16_t  dig_P7 = 0;
        int16_t  dig_P8 = 0;
        int16_t  dig_P9 = 0;

        uint8_t  dig_H1 = 0;
        int16_t  dig_H2 = 0;
        uint8_t  dig_H3 = 0;
        int16_t  dig_H4 = 0;
        int16_t  dig_H5 = 0;
        int8_t   dig_H6 = 0;
    };

    std::string i2c_device_;
    uint8_t address_;
    int fd_ = -1;
    CalibrationData calib_{};
    int32_t t_fine_ = 0;

    bool openDevice();
    void closeDevice();

    bool readCalibration();
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t start_reg, uint8_t* buffer, size_t length);
    uint8_t readChipId();

    double compensateTemperature(int32_t adc_T);
    double compensatePressure(int32_t adc_P);
    double compensateHumidity(int32_t adc_H);
};
