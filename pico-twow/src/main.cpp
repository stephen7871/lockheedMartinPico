#include "app/packet_builder.hpp"
#include "config.hpp"
#include "hardware/spi_demo.hpp"
#include "network/udp_sender.hpp"
#include "sensor/bme280_driver.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./pi_zero_telemetry <MAC_IP> [UDP_PORT]\n";
        return 1;
    }

    const std::string mac_ip = argv[1];
    const uint16_t udp_port =
        (argc >= 3) ? static_cast<uint16_t>(std::stoi(argv[2])) : config::UDP_PORT;

    std::cout << "Starting telemetry app\n";
    std::cout << "Destination: " << mac_ip << ":" << udp_port << "\n";

    // Try BME280 on 0x76, then 0x77
    BME280Driver sensor_primary(config::I2C_DEVICE, config::BME280_ADDR_PRIMARY);
    BME280Driver sensor_secondary(config::I2C_DEVICE, config::BME280_ADDR_SECONDARY);

    BME280Driver* active_sensor = nullptr;

    if (sensor_primary.initialize()) {
        active_sensor = &sensor_primary;
        std::cout << "BME280 initialized at I2C address 0x76\n";
    } else if (sensor_secondary.initialize()) {
        active_sensor = &sensor_secondary;
        std::cout << "BME280 initialized at I2C address 0x77\n";
    } else {
        std::cerr << "Failed to initialize BME280 on 0x76 or 0x77\n";
        return 1;
    }

    UdpSender sender(mac_ip, udp_port);
    if (!sender.initialize()) {
        std::cerr << "Failed to initialize UDP sender\n";
        return 1;
    }

    SPIDemo spi(
        config::SPI_DEVICE,
        config::SPI_SPEED_HZ,
        config::SPI_MODE,
        config::SPI_BITS_PER_WORD
    );

    bool spi_ready = spi.initialize();
    if (spi_ready) {
        std::cout << "SPI initialized on " << config::SPI_DEVICE << "\n";
        std::cout << "Optional self-test: jumper MOSI to MISO for loopback verification\n";
    } else {
        std::cout << "SPI initialization failed. Continuing with I2C + UDP demo.\n";
    }

    uint64_t sequence = 0;

    while (true) {
        try {
            SensorReading reading = active_sensor->read();

            bool spi_test_ran = false;
            bool spi_test_passed = false;
            std::vector<uint8_t> spi_tx;
            std::vector<uint8_t> spi_rx;

            if (spi_ready && (sequence % config::SPI_DEMO_EVERY_N_LOOPS == 0)) {
                spi_test_ran = true;
                spi_tx = {0xAA, 0x55, 0x0F, 0xF0};
                spi_rx = spi.transfer(spi_tx);
                spi_test_passed = (!spi_rx.empty() && spi_rx == spi_tx);
            }

            std::string payload = PacketBuilder::buildTelemetryJson(
                sequence,
                reading,
                spi_test_ran,
                spi_test_passed,
                spi_tx,
                spi_rx
            );

            bool ok = sender.send(payload);

            std::cout << "seq=" << sequence
                      << " temp=" << reading.temperature_c << "C"
                      << " hum=" << reading.humidity_percent << "%"
                      << " pressure=" << reading.pressure_hpa << "hPa"
                      << " udp=" << (ok ? "sent" : "failed")
                      << "\n";

            if (spi_test_ran) {
                std::cout << "SPI demo ran, loopback result="
                          << (spi_test_passed ? "PASS" : "NO LOOPBACK / FAIL")
                          << "\n";
            }

            ++sequence;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config::TELEMETRY_INTERVAL_MS)
            );
        } catch (const std::exception& ex) {
            std::cerr << "Read/send error: " << ex.what() << "\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    return 0;
}
