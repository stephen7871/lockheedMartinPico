import json
import socket
import sys
from datetime import datetime

HOST = "0.0.0.0"
PORT = 5005

if len(sys.argv) >= 2:
    PORT = int(sys.argv[1])

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))

print(f"Listening on UDP {HOST}:{PORT}")

while True:
    data, addr = sock.recvfrom(4096)
    text = data.decode("utf-8", errors="replace")

    print("\n" + "=" * 70)
    print(f"Time: {datetime.now().isoformat(timespec='seconds')}")
    print(f"From: {addr[0]}:{addr[1]}")

    try:
        packet = json.loads(text)
        print("Sequence:", packet.get("sequence"))
        print("Hardware protocols:", packet.get("protocols", {}).get("hardware"))
        print("Network protocols:", packet.get("protocols", {}).get("network"))

        sensor = packet.get("sensor", {})
        print(f"Temperature: {sensor.get('temperature_c')} C")
        print(f"Humidity:    {sensor.get('humidity_percent')} %")
        print(f"Pressure:    {sensor.get('pressure_hpa')} hPa")

        spi = packet.get("spi_demo", {})
        print("SPI ran:", spi.get("ran"))
        print("SPI passed:", spi.get("passed"))
        print("SPI TX:", spi.get("tx"))
        print("SPI RX:", spi.get("rx"))
    except json.JSONDecodeError:
        print("Raw payload:")
        print(text)
