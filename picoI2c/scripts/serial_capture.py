import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Install pyserial first:")
    print("python3 -m pip install pyserial")
    sys.exit(1)


def find_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        name = (port.device or "") + " " + (port.description or "")
        if "usbmodem" in name.lower() or "board in fs mode" in name.lower():
            return port.device
    return None


def main():
    port = find_port()
    if not port:
        print("No Pico serial port found. Plug in the board and make sure firmware is running.")
        return

    print(f"Opening {port} ...")
    with serial.Serial(port, 115200, timeout=1) as ser:
        time.sleep(2)
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if line:
                print(line)


if __name__ == "__main__":
    main()