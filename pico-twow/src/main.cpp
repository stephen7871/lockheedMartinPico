#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "app/packet_builder.hpp"
#include "hw/i2c_bus.hpp"
#include "hw/spi_bus.hpp"
#include "net/udp_client.hpp"
#include "sensor/bme280_driver.hpp"

// Implemented in the Linux source files
II2CBus* create_linux_i2c_bus(const std::string& device_path);
ISpiBus* create_linux_spi_bus(const std::string& device_path);

namespace {
void print_usage() {
    std::cout
        << "Usage:\n"
        << "  ./protocol_demo i2c <i2c_dev> <hex_addr> <target_ip> <target_port>\n"
        << "  ./protocol_demo spi <spi_dev> <target_ip> <target_port>\n\n"
        << "Examples:\n"
        << "  ./protocol_demo i2c /dev/i2c-1 0x76 192.168.1.20 5000\n"
        << "  ./protocol_demo spi /dev/spidev0.0 192.168.1.20 5000\n";
}
}

int main(int argc, char** argv) {
    try {
        if (argc < 5) {
            print_usage();
            return 1;
        }

        const std::string mode = argv[1];
        UdpClient udp;

        if (mode == "i2c") {
            if (argc != 6) {
                print_usage();
                return 1;
            }

            const std::string i2c_dev = argv[2];
            const auto addr = static_cast<uint8_t>(std::stoul(argv[3], nullptr, 16));
            const std::string target_ip = argv[4];
            const auto target_port = static_cast<uint16_t>(std::stoi(argv[5]));

            std::unique_ptr<II2CBus> bus(create_linux_i2c_bus(i2c_dev));
            Bme280I2cDriver driver(*bus, addr);

            if (!driver.initialize()) {
                std::cerr << "Failed to initialize BME280 over I2C\n";
                return 2;
            }

            for (int i = 0; i < 10; ++i) {
                const auto chip_id = driver.read_chip_id();
                const auto sample = driver.read_raw_sample();
                const auto packet = build_udp_packet("i2c", sample, chip_id);

                std::cout << packet << '\n';
                if (!udp.send_to(target_ip, target_port, packet)) {
                    std::cerr << "UDP send failed\n";
                    return 3;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else if (mode == "spi") {
            if (argc != 5) {
                print_usage();
                return 1;
            }

            const std::string spi_dev = argv[2];
            const std::string target_ip = argv[3];
            const auto target_port = static_cast<uint16_t>(std::stoi(argv[4]));

            std::unique_ptr<ISpiBus> bus(create_linux_spi_bus(spi_dev));
            Bme280SpiDriver driver(*bus);

            if (!driver.initialize()) {
                std::cerr << "Failed to initialize BME280 over SPI\n";
                return 2;
            }

            for (int i = 0; i < 10; ++i) {
                const auto chip_id = driver.read_chip_id();
                const auto sample = driver.read_raw_sample();
                const auto packet = build_udp_packet("spi", sample, chip_id);

                std::cout << packet << '\n';
                if (!udp.send_to(target_ip, target_port, packet)) {
                    std::cerr << "UDP send failed\n";
                    return 3;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            print_usage();
            return 1;
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 99;
    }
}