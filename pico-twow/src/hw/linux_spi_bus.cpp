#include "hw/spi_bus.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

class LinuxSpiBus final : public ISpiBus {
public:
    explicit LinuxSpiBus(const std::string& device_path)
        : fd_(-1) {
        fd_ = ::open(device_path.c_str(), O_RDWR);
        if (fd_ < 0) {
            throw std::runtime_error("Failed to open SPI device: " + device_path);
        }

        uint8_t mode = SPI_MODE_0;
        uint8_t bits = 8;
        uint32_t speed = 500000;

        if (::ioctl(fd_, SPI_IOC_WR_MODE, &mode) < 0) {
            throw std::runtime_error("Failed to set SPI mode");
        }
        if (::ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
            throw std::runtime_error("Failed to set SPI bits per word");
        }
        if (::ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            throw std::runtime_error("Failed to set SPI speed");
        }
    }

    ~LinuxSpiBus() override {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx) override {
        std::vector<uint8_t> rx(tx.size(), 0);

        spi_ioc_transfer tr{};
        tr.tx_buf = reinterpret_cast<unsigned long>(tx.data());
        tr.rx_buf = reinterpret_cast<unsigned long>(rx.data());
        tr.len = static_cast<uint32_t>(tx.size());
        tr.delay_usecs = 0;
        tr.speed_hz = 500000;
        tr.bits_per_word = 8;

        if (::ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) < 0) {
            throw std::runtime_error("SPI transfer failed");
        }

        return rx;
    }

private:
    int fd_;
};

// Factory-style helper exposed to main.cpp via forward declaration.
ISpiBus* create_linux_spi_bus(const std::string& device_path) {
    return new LinuxSpiBus(device_path);
}