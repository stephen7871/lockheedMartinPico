import json
import socket

HOST = "0.0.0.0"
PORT = 5000

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((HOST, PORT))

    print(f"Listening for UDP packets on {HOST}:{PORT}")

    while True:
        data, addr = sock.recvfrom(4096)
        text = data.decode("utf-8", errors="replace")
        print(f"\nFrom {addr[0]}:{addr[1]}")
        print(f"Raw: {text}")

        try:
            packet = json.loads(text)
            print("Parsed JSON:")
            for key, value in packet.items():
                print(f"  {key}: {value}")
        except json.JSONDecodeError:
            print("Payload was not valid JSON")

if __name__ == "__main__":
    main()