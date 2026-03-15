# Raspberry Pi Zero 2 W Telemetry Demo
I2C + SPI + UDP/IP using C++ and Python

This project demonstrates hardware communication and network telemetry using a Raspberry Pi Zero 2 W and a BME280 environmental sensor.

The Raspberry Pi reads temperature, humidity, and pressure from the sensor over **I2C**, initializes **SPI**, and sends telemetry data over **UDP/IP** to a Python listener running on another computer.

---

# Protocols Demonstrated

## Hardware Protocols

### I2C (Inter-Integrated Circuit)

Used to communicate with the **BME280 environmental sensor**.

Demonstrates:

- I2C device addressing
- Register reads
- Linux device interface

Device used:

```
/dev/i2c-1
```

Check sensor detection (on Pico via ssh):

```
i2cdetect -y 1
```

Expected output contains:

```
70: -- -- -- -- -- -- 76 --
```

Address `0x76` indicates the BME280 sensor is detected.

---

### SPI (Serial Peripheral Interface)

The application initializes SPI to demonstrate a second hardware communication protocol.

Demonstrates:

- SPI initialization
- SPI transfer interface
- Linux spidev driver usage

Device used:

```
/dev/spidev0.0
```

---

## Networking Protocols

### IP (Internet Protocol)

Used for addressing devices on the network.

Example destination:

```
YOUR_IP
```

---

### UDP (User Datagram Protocol)

Telemetry packets are sent from the Raspberry Pi to another computer using UDP sockets.

---

# Languages Used

### C++

Used for:

- sensor driver
- hardware protocol communication
- UDP telemetry sender

### Python

Used for:

- UDP listener
- telemetry packet parsing
- printing received sensor data

---

# How to Run the Project

The system consists of **two programs**:

1. Python UDP listener (runs on your computer)
2. C++ telemetry sender (runs on the Raspberry Pi)

---

# Step 1 — Find your computer's IP address

On your computer run:

```
ipconfig getifaddr en0
```

Example output:

```
YOUR_IP
```

---

# Step 2 — Start the UDP listener (on your computer)

Run:

```
python3 /Users/stephenmcnally/lockheedInterview/pico-twow/scripts/udp_listener.py 5005
```

You should see:

```
Listening on UDP port 5005
```

Leave this terminal open.

---

# Step 3 — Build the project on the Raspberry Pi

Run on the Pi:

```
cd ~/interview/pico-twow
mkdir -p build
cd build
cmake ..
make -j2
```

---

# Step 4 — Run the telemetry program on the Raspberry Pi

```
cd ~/interview/pico-twow/build
./pi_zero_telemetry YOUR_IP 5005
```

Replace `YOUR_IP` with your computer's current IP address.

---

# Expected Output

## Raspberry Pi

```
Starting telemetry app
Destination: YOUR_IP:5005
BME280 initialized at I2C address 0x76
SPI initialized on /dev/spidev0.0
seq=0 temp=21.8C hum=33.5% pressure=960hPa udp=sent
```

---

## Computer

```
Sequence: 0
Hardware protocols: ['I2C', 'SPI']
Network protocols: ['IP', 'UDP']

Temperature: 21.8 C
Humidity: 33.5 %
Pressure: 960 hPa
```

---

# Summary

This project demonstrates:

Hardware protocols

- I2C
- SPI

Networking protocols

- IP
- UDP

Languages

- C++
- Python
