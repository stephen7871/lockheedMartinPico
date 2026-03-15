#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SPIDemo {
public:
    SPIDemo(const std::string& device, uint32_t speed_hz, uint8_t mode, uint8_t bits_per_word);
    ~SPIDemo();

    bool initialize();
    std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx_bytes);
    bool loopbackSelfTest();

private:
    std::string device_;
    uint32_t speed_hz_;
    uint8_t mode_;
    uint8_t bits_per_word_;
    int fd_ = -1;

    void closeDevice();
};
