#pragma once

#include <cstdint>
#include <vector>

class II2CBus {
public:
    virtual ~II2CBus() = default;

    virtual uint8_t read_byte(uint8_t device_addr, uint8_t reg) = 0;
    virtual std::vector<uint8_t> read_bytes(uint8_t device_addr, uint8_t start_reg, std::size_t count) = 0;
    virtual void write_byte(uint8_t device_addr, uint8_t reg, uint8_t value) = 0;
};