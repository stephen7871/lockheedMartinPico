#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "bme280/bme280_driver.hpp"
#include "protocols/i2c_bus.hpp"

class PicoI2cBus : public II2cBus {
public:
    explicit PicoI2cBus(i2c_inst_t* instance) : instance_(instance) {}

    bool write(uint8_t address, const uint8_t* data, std::size_t length) override {
        const int written =
            i2c_write_blocking(instance_, address, data, static_cast<size_t>(length), false);
        return written == static_cast<int>(length);
    }

    bool write_read(uint8_t address,
                    const uint8_t* write_data,
                    std::size_t write_length,
                    uint8_t* read_data,
                    std::size_t read_length) override {
        const int written =
            i2c_write_blocking(instance_, address, write_data, static_cast<size_t>(write_length), true);
        if (written != static_cast<int>(write_length)) {
            return false;
        }

        const int read =
            i2c_read_blocking(instance_, address, read_data, static_cast<size_t>(read_length), false);
        return read == static_cast<int>(read_length);
    }

private:
    i2c_inst_t* instance_;
};

static void setup_i2c() {
    i2c_init(i2c0, 100 * 1000);

    // Your code expects:
    // SDA = GPIO4 (Pico pin 6)
    // SCL = GPIO5 (Pico pin 7)
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
}

static void check_i2c_line_levels() {
    printf("Checking raw I2C line levels before I2C peripheral setup...\n");

    gpio_init(4);
    gpio_init(5);
    gpio_set_dir(4, GPIO_IN);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(4);
    gpio_pull_up(5);

    sleep_ms(10);

    const int sda = gpio_get(4);
    const int scl = gpio_get(5);

    printf("GPIO4 / SDA level: %d\n", sda);
    printf("GPIO5 / SCL level: %d\n", scl);

    if (sda == 0 || scl == 0) {
        printf("WARNING: One or both I2C lines are being held LOW.\n");
        printf("Possible causes: bad wiring, short, loose contact, or sensor not seated correctly.\n");
    } else {
        printf("Both I2C lines are HIGH. Pull-ups appear present.\n");
    }
}

static void scan_i2c() {
    printf("Scanning I2C bus with timeouts...\n");

    for (uint8_t addr = 0x08; addr < 0x78; ++addr) {
        uint8_t dummy = 0;
        int result = i2c_read_timeout_us(i2c0, addr, &dummy, 1, false, 2000);

        if (result >= 0) {
            printf("Found device at 0x%02X\n", addr);
        }
    }

    printf("Scan complete\n");
}

int main() {
    const unsigned int LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(10000);  // gives host time to open serial

    printf("Starting Pico 2 BME280 I2C demo...\n");
    printf("Expected wiring:\n");
    printf("  VIN -> Pico pin 1 (3V3)\n");
    printf("  GND -> Pico pin 3 (GND)\n");
    printf("  SDA -> Pico pin 6 (GPIO4)\n");
    printf("  SCL -> Pico pin 7 (GPIO5)\n");

    check_i2c_line_levels();
    setup_i2c();
    scan_i2c();

    PicoI2cBus bus(i2c0);

    Bme280Driver sensor76(bus, 0x76);
    Bme280Driver sensor77(bus, 0x77);

    Bme280Driver* active = nullptr;

    if (sensor76.init()) {
        active = &sensor76;
        printf("BME280 found at address 0x76\n");
    } else if (sensor77.init()) {
        active = &sensor77;
        printf("BME280 found at address 0x77\n");
    } else {
        printf("Failed to initialize BME280 on 0x76 or 0x77\n");
    }

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);

        if (active) {
            Bme280Sample sample{};
            if (active->read_sample(sample)) {
                printf("Temp: %.2f C | Pressure: %.2f hPa | Humidity: %.2f %%\n",
                       sample.temperature_c,
                       sample.pressure_hpa,
                       sample.humidity_percent);
            } else {
                printf("Sample read failed\n");
            }
        }
    }

    return 0;
}