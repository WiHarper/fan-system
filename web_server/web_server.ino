#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// ---------------- W5500 pins for ESP32-S3 ----------------
#define W5500_MISO 12
#define W5500_MOSI 11
#define W5500_SCLK 13
#define W5500_CS   14
#define W5500_RST  9

// ---------------- PWM Output Pins ----------------
#define FAN1_PIN   15
#define SERVO1_PIN 16
#define SERVO2_PIN 17

// ---------------- Network settings ----------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 50, 100);
unsigned int localPort = 8888;
EthernetUDP Udp;

byte packetBuffer[3];

// ---------------- PWM Setup Functions ----------------
bool setupFan(int pin) {
  // Fans like higher frequencies. 10 kHz, 8-bit resolution (0-255)
  bool ok = ledcAttach(pin, 10000, 8);
  if (!ok) return false;
  ledcWrite(pin, 0); // Start off
  return true;
}

bool setupServo(int pin) {
  // Servos REQUIRE 50 Hz. ESP32-S3 supports a MAX of 14-bit resolution!
  bool ok = ledcAttach(pin, 50, 14);
  if (!ok) return false;
  
  // Start servo at middle position (~1.5ms pulse)
  // 14-bit max is 16384 steps. At 50Hz (20ms period), 1.5ms is roughly 1229 steps.
  ledcWrite(pin, 1229); 
  return true;
}

void applyPwmStates(uint8_t fan1, uint8_t servo1, uint8_t servo2) {
  // 1. Fan uses direct 8-bit duty cycle (0-255)
  ledcWrite(FAN1_PIN, fan1);

  // 2. Servos map the 0-255 byte to 14-bit duty cycles
  // A standard servo expects a 1ms (0°) to 2ms (180°) pulse over a 20ms period.
  // 20ms period at 14-bit = 16384 steps. 
  // 1ms pulse = ~819 steps. 2ms pulse = ~1638 steps.
  uint32_t s1_duty = map(servo1, 0, 255, 819, 1638);
  uint32_t s2_duty = map(servo2, 0, 255, 819, 1638);

  ledcWrite(SERVO1_PIN, s1_duty);
  ledcWrite(SERVO2_PIN, s2_duty);

  Serial.print("Outputs -> Fan: "); Serial.print(fan1);
  Serial.print(" | Servo 1: "); Serial.print(servo1);
  Serial.print(" | Servo 2: "); Serial.println(servo2);
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Reset W5500
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(10);
  digitalWrite(W5500_RST, HIGH);
  delay(100);

  // Start SPI and Ethernet
  SPI.begin(W5500_SCLK, W5500_MISO, W5500_MOSI, W5500_CS);
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Error: W5500 module was not found.");
    while (true) { delay(1); }
  }

  Udp.begin(localPort);

  // Set up PWM outputs safely
  if (!setupFan(FAN1_PIN) || !setupServo(SERVO1_PIN) || !setupServo(SERVO2_PIN)) {
    Serial.println("One or more PWM pins failed to initialize.");
    while (true) { delay(1); }
  }

  Serial.println("UDP Receiver Ready!");
}

void loop() {
  int packetSize = Udp.parsePacket();

  if (packetSize >= 3) {
    Udp.read(packetBuffer, 3);

    // Flush any remaining bytes
    while (Udp.available()) Udp.read();

    applyPwmStates(packetBuffer[0], packetBuffer[1], packetBuffer[2]);
  } else if (packetSize > 0) {
    while (Udp.available()) Udp.read();
    Serial.println("Packet too small. Ignored.");
  }
}