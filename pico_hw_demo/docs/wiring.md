# Wiring Notes

## I2C

Use these pins on the Pico 2:

- GP4 -> SDA
- GP5 -> SCL
- 3V3(OUT) -> VIN
- GND -> GND

If your module has `SDO` and `CS` pins but you are using I2C:
- Leave `CS` tied high if your board requires it for I2C mode
- `SDO` may select the I2C address (`0x76` vs `0x77`) depending on the breakout design

Default in this project: address `0x76`

If your board responds at `0x77`, update the constant in `main.cpp`.

## SPI

Use these pins on the Pico 2:

- GP18 -> SCK
- GP19 -> MOSI / SDI
- GP16 -> MISO / SDO
- GP17 -> CS
- 3V3(OUT) -> VIN
- GND -> GND

## Bring-up checklist

1. Verify power and ground first
2. Start with I2C because it usually has fewer wires
3. Confirm chip ID register returns `0x60`
4. Then read compensated sensor values
5. After I2C works, rewire for SPI and confirm the same sensor can be read over the second protocol
