# Pico 2 + BME280 Hardware Protocol Demo

This project demonstrates:

- Embedded C++
- I2C hardware communication with a BME280
- Raspberry Pi Pico 2 firmware build with CMake/Make
- Python serial capture script
- GoogleTest unit and integration tests on host

## Wiring (I2C)

Pico 2 -> BME280

- 3.3V -> VIN
- GND -> GND
- GP4 -> SDA
- GP5 -> SCL

## Firmware build

```bash
export PICO_SDK_PATH=~/LOCKHEEDINTERVIEW/pico-sdk
rm -rf build
mkdir build
cd build
cmake ..
make -j4