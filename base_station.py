import socket
import time

UDP_IP = "192.168.50.100"
UDP_PORT = 8888

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def send_pwm(fan1, fan2, fan3):
    f1 = max(0, min(255, int(fan1)))
    f2 = max(0, min(255, int(fan2)))
    f3 = max(0, min(255, int(fan3)))

    payload = bytes([f1, f2, f3])
    sock.sendto(payload, (UDP_IP, UDP_PORT))

index = 0
if __name__ == "__main__":
    print(f"Targeting {UDP_IP}:{UDP_PORT}...")
    while True:
        send_pwm(index % 256, 2, 3)
        time.sleep(0.01)
        index += 1