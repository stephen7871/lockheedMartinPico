#include "protocols/i2c_transport_pico.hpp"

#include <vector>

#ifdef PICO_ON_DEVICE
#include "hardware/i2c.h"
#endif

#ifdef PICO_ON_DEVICE
I2cTransportPico::I2cTransportPico(i2c_inst_t* instance, std::uint8_t device_address)
    : instance_(instance), device_address_(device_address) {}
#else
I2cTransportPico::I2cTransportPico(void* instance, std::uint8_t device_address)
    : instance_(instance), device_address_(device_address) {}
#endif

bool I2cTransportPico::write_register(std::uint8_t reg, const std::uint8_t* data, std::size_t length) {
#ifdef PICO_ON_DEVICE
    std::vector<std::uint8_t> buffer(length + 1);
    buffer[0] = reg;
    for (std::size_t i = 0; i < length; ++i) {
        buffer[i + 1] = data[i];
    }
    const int written = i2c_write_blocking(instance_, device_address_, buffer.data(), static_cast<int>(buffer.size()), false);
    return written == static_cast<int>(buffer.size());
#else
    (void)reg;
    (void)data;
    (void)length;
    return false;
#endif
}

bool I2cTransportPico::read_register(std::uint8_t reg, std::uint8_t* data, std::size_t length) {
#ifdef PICO_ON_DEVICE
    const int written = i2c_write_blocking(instance_, device_address_, &reg, 1, true);
    if (written != 1) {
        return false;
    }
    const int read = i2c_read_blocking(instance_, device_address_, data, static_cast<int>(length), false);
    return read == static_cast<int>(length);
#else
    (void)reg;
    (void)data;
    (void)length;
    return false;
#endif
}
