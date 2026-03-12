#pragma once

#include <cstddef>
#include <cstdint>

class II2cBus {
public:
    virtual ~II2cBus() = default;

    virtual bool write(uint8_t address, const uint8_t* data, std::size_t length) = 0;

    virtual bool write_read(uint8_t address,
                            const uint8_t* write_data,
                            std::size_t write_length,
                            uint8_t* read_data,
                            std::size_t read_length) = 0;
};