#include "hw/i2c_bus.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

class LinuxI2CBus final : public II2CBus {
public:
    explicit LinuxI2CBus(const std::string& device_path)
        : fd_(-1) {
        fd_ = ::open(device_path.c_str(), O_RDWR);
        if (fd_ < 0) {
            throw std::runtime_error("Failed to open I2C device: " + device_path);
        }
    }

    ~LinuxI2CBus() override {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    uint8_t read_byte(uint8_t device_addr, uint8_t reg) override {
        set_slave(device_addr);

        if (::write(fd_, &reg, 1) != 1) {
            throw std::runtime_error("I2C write register address failed");
        }

        uint8_t value{};
        if (::read(fd_, &value, 1) != 1) {
            throw std::runtime_error("I2C read byte failed");
        }

        return value;
    }

    std::vector<uint8_t> read_bytes(uint8_t device_addr, uint8_t start_reg, std::size_t count) override {
        set_slave(device_addr);

        if (::write(fd_, &start_reg, 1) != 1) {
            throw std::runtime_error("I2C write start register failed");
        }

        std::vector<uint8_t> data(count);
        if (::read(fd_, data.data(), count) != static_cast<ssize_t>(count)) {
            throw std::runtime_error("I2C block read failed");
        }

        return data;
    }

    void write_byte(uint8_t device_addr, uint8_t reg, uint8_t value) override {
        set_slave(device_addr);

        uint8_t buffer[2] = {reg, value};
        if (::write(fd_, buffer, 2) != 2) {
            throw std::runtime_error("I2C write byte failed");
        }
    }

private:
    void set_slave(uint8_t device_addr) {
        if (::ioctl(fd_, I2C_SLAVE, device_addr) < 0) {
            throw std::runtime_error("Failed to set I2C slave address");
        }
    }

    int fd_;
};

// Factory-style helper exposed to main.cpp via forward declaration.
II2CBus* create_linux_i2c_bus(const std::string& device_path) {
    return new LinuxI2CBus(device_path);
}