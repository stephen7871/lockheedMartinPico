#pragma once

#include <cstdint>
#include <vector>

class ISpiBus {
public:
    virtual ~ISpiBus() = default;

    virtual std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx) = 0;
};