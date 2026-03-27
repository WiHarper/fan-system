#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h> // We need the UDP library now

// Define your specific ESP32-S3 pins
#define W5500_MISO 12
#define W5500_MOSI 11
#define W5500_SCLK 13
#define W5500_CS   14
#define W5500_RST  9

// Network settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 50, 100); 
unsigned int localPort = 8888; // The port we will listen on

// Create a UDP instance
EthernetUDP Udp;

// A buffer to hold incoming packets (we only expect 3 bytes)
byte packetBuffer[3]; 

void setup() {
  Serial.begin(115200);
  delay(3000); 
  
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(10);
  digitalWrite(W5500_RST, HIGH);
  delay(100); 

  // Start the SPI bus and Ethernet
  SPI.begin(W5500_SCLK, W5500_MISO, W5500_MOSI, W5500_CS);
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Error: W5500 module was not found.");
    while (true) { delay(1); } 
  }

  // Start listening for UDP packets
  Udp.begin(localPort);
  
  Serial.println("UDP Receiver Started!");
  Serial.print("Listening at IP: ");
  Serial.print(Ethernet.localIP());
  Serial.print(" on Port: ");
  Serial.println(localPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    Udp.read(packetBuffer, 3);
    
    Serial.print("Received UDP PWM -> ");
    Serial.print("Fan 1: "); Serial.print(packetBuffer[0]);
    Serial.print(" | Fan 2: "); Serial.print(packetBuffer[1]);
    Serial.print(" | Fan 3: "); Serial.println(packetBuffer[2]);
  }
}