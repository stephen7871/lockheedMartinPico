#include "hardware/spi_demo.hpp"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

SPIDemo::SPIDemo(const std::string& device, uint32_t speed_hz, uint8_t mode, uint8_t bits_per_word)
    : device_(device), speed_hz_(speed_hz), mode_(mode), bits_per_word_(bits_per_word) {}

SPIDemo::~SPIDemo() {
    closeDevice();
}

void SPIDemo::closeDevice() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool SPIDemo::initialize() {
    fd_ = ::open(device_.c_str(), O_RDWR);
    if (fd_ < 0) {
        std::perror("open spi");
        return false;
    }

    if (ioctl(fd_, SPI_IOC_WR_MODE, &mode_) < 0) {
        std::perror("SPI_IOC_WR_MODE");
        return false;
    }

    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word_) < 0) {
        std::perror("SPI_IOC_WR_BITS_PER_WORD");
        return false;
    }

    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz_) < 0) {
        std::perror("SPI_IOC_WR_MAX_SPEED_HZ");
        return false;
    }

    return true;
}

std::vector<uint8_t> SPIDemo::transfer(const std::vector<uint8_t>& tx_bytes) {
    std::vector<uint8_t> rx_bytes(tx_bytes.size(), 0);

    spi_ioc_transfer tr{};
    tr.tx_buf = reinterpret_cast<unsigned long>(tx_bytes.data());
    tr.rx_buf = reinterpret_cast<unsigned long>(rx_bytes.data());
    tr.len = static_cast<uint32_t>(tx_bytes.size());
    tr.speed_hz = speed_hz_;
    tr.bits_per_word = bits_per_word_;

    if (ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) < 1) {
        std::perror("SPI_IOC_MESSAGE");
        return {};
    }

    return rx_bytes;
}

bool SPIDemo::loopbackSelfTest() {
    // For a true self-test, connect MOSI to MISO with a jumper.
    std::vector<uint8_t> tx = {0xAA, 0x55, 0x0F, 0xF0};
    std::vector<uint8_t> rx = transfer(tx);
    return !rx.empty() && rx == tx;
}
