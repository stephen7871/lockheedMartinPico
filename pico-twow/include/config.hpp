#pragma once

#include <cstdint>
#include <string>

namespace config {

// ---- I2C / BME280 ----
static constexpr const char* I2C_DEVICE = "/dev/i2c-1";
static constexpr uint8_t BME280_ADDR_PRIMARY = 0x76;
static constexpr uint8_t BME280_ADDR_SECONDARY = 0x77;

// ---- SPI ----
static constexpr const char* SPI_DEVICE = "/dev/spidev0.0";
static constexpr uint32_t SPI_SPEED_HZ = 500000;
static constexpr uint8_t SPI_MODE = 0;
static constexpr uint8_t SPI_BITS_PER_WORD = 8;

// ---- UDP ----
static constexpr uint16_t UDP_PORT = 5005;

// ---- App ----
static constexpr int TELEMETRY_INTERVAL_MS = 2000;
static constexpr int SPI_DEMO_EVERY_N_LOOPS = 3;

} // namespace config
