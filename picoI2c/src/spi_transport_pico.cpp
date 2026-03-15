#include "protocols/spi_transport_pico.hpp"

#ifdef PICO_ON_DEVICE
#include "hardware/gpio.h"
#include "hardware/spi.h"
#endif

#include <vector>

#ifdef PICO_ON_DEVICE
SpiTransportPico::SpiTransportPico(spi_inst_t* instance, std::uint cs_pin)
    : instance_(instance), cs_pin_(cs_pin) {}
#else
SpiTransportPico::SpiTransportPico(void* instance, unsigned int cs_pin)
    : instance_(instance), cs_pin_(cs_pin) {}
#endif

void SpiTransportPico::select() {
#ifdef PICO_ON_DEVICE
    gpio_put(cs_pin_, 0);
#endif
}

void SpiTransportPico::deselect() {
#ifdef PICO_ON_DEVICE
    gpio_put(cs_pin_, 1);
#endif
}

bool SpiTransportPico::write_register(std::uint8_t reg, const std::uint8_t* data, std::size_t length) {
#ifdef PICO_ON_DEVICE
    std::vector<std::uint8_t> buffer(length + 1);
    buffer[0] = static_cast<std::uint8_t>(reg & 0x7F);
    for (std::size_t i = 0; i < length; ++i) {
        buffer[i + 1] = data[i];
    }

    select();
    const int written = spi_write_blocking(instance_, buffer.data(), static_cast<int>(buffer.size()));
    deselect();
    return written == static_cast<int>(buffer.size());
#else
    (void)reg;
    (void)data;
    (void)length;
    return false;
#endif
}

bool SpiTransportPico::read_register(std::uint8_t reg, std::uint8_t* data, std::size_t length) {
#ifdef PICO_ON_DEVICE
    const std::uint8_t addr = static_cast<std::uint8_t>(reg | 0x80);
    select();
    const int addr_written = spi_write_blocking(instance_, &addr, 1);
    if (addr_written != 1) {
        deselect();
        return false;
    }
    const int read = spi_read_blocking(instance_, 0x00, data, static_cast<int>(length));
    deselect();
    return read == static_cast<int>(length);
#else
    (void)reg;
    (void)data;
    (void)length;
    return false;
#endif
}
