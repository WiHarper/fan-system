#include <SPI.h>
#include <Ethernet.h>

// Define your specific ESP32-S3 pins
#define W5500_MISO 12
#define W5500_MOSI 11
#define W5500_SCLK 13
#define W5500_CS   14
#define W5500_RST  9

// The pin for your LED
#define LED_PIN 4 

// Dummy MAC address
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Define a Static IP for the ESP32
IPAddress ip(192, 168, 50, 100); 

// Create a server listening on port 80 (standard HTTP port)
EthernetServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED off
  delay(3000); 
  
  // Hard reset the W5500 module
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

  // Start the web server
  server.begin();
  
  Serial.print("Web Server Started! Open a browser and go to: http://");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Listen for incoming clients (like your web browser)
  EthernetClient client = server.available();
  
  if (client) {
    String currentLine = ""; 
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // If we get a newline character, the HTTP request is ending
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send a standard HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Build the HTML Webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>body { text-align: center; font-family: Arial; margin-top: 50px;} ");
            client.println(".btn { background-color: #4CAF50; color: white; padding: 15px 32px; text-decoration: none; font-size: 24px; margin: 10px; cursor: pointer; border: none; border-radius: 8px;} ");
            client.println(".btn-off { background-color: #f44336; }</style></head>");
            client.println("<body><h1>ESP32 Control Panel</h1>");
            
            // Add the ON and OFF buttons
            client.println("<p><a href=\"/LED=ON\"><button class=\"btn\">Turn ON</button></a></p>");
            client.println("<p><a href=\"/LED=OFF\"><button class=\"btn btn-off\">Turn OFF</button></a></p>");
            client.println("</body></html>");
            
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c; // Add to the end of the line
        }

        // Check the URL request to see if a button was clicked
        if (currentLine.endsWith("GET /LED=ON")) {
          digitalWrite(LED_PIN, HIGH);
          Serial.println("Command Received: LED ON");
        }
        if (currentLine.endsWith("GET /LED=OFF")) {
          digitalWrite(LED_PIN, LOW);
          Serial.println("Command Received: LED OFF");
        }
      }
    }
    // Give the browser time to receive the data, then close the connection
    delay(1);
    client.stop();
  }
}