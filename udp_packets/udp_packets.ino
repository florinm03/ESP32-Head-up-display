// UDP Packets getter from BeamNG for the Arduino IDE

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>


// Network credentials
const char* ssid = "Network Name";
const char* password = "password";

// Correct pin definitions for ESP32-2432S028R
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

TFT_eSPI tft = TFT_eSPI();

// UDP setup for receiving BeamNG data
WiFiUDP udp;
const int udpPort = 4444;

// Car data structure
struct CarData {
  float speed;     // km/h
  int rpm;         // RPM
  int gear;        // Current gear
  float fuel;      // Fuel percentage
  float throttle;  // Throttle position (0-1)
  float brake;     // Brake position (0-1)
  float steering;  // Steering angle
};

CarData carData = { 0, 0, 0, 100.0, 0, 0, 0 };


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

// Calibration values
#define X_MIN 200
#define X_MAX 3700
#define Y_MIN 240
#define Y_MAX 3800

void screenSetup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32-2432S028R System Monitor...");

  // Turn on backlight
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  Serial.println("Backlight should be ON");

  // Initialize the display
  tft.init();
  tft.setRotation(1);  // Landscape mode
  Serial.println("TFT initialized");

  // Clear screen with black background
  tft.fillScreen(TFT_BLACK);

  // Title
  // tft.setTextColor(TFT_CYAN, TFT_BLACK);
  // tft.setTextSize(2);
  // tft.setCursor(10, 10);
  // tft.println("Welcome");

  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  tft.drawCentreString("Welcome", centerX, 30, FONT_SIZE);

  delay(1000);  // Give time for initialization
}

void initWiFi() {
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  tft.fillScreen(TFT_BLACK);
  tft.println("Connecting to WiFi ");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    tft.print('.');
    delay(1000);
  }
  tft.fillScreen(TFT_BLACK);
  tft.drawString("WiFi Connected!", 10, 10);
  tft.drawString("IP: " + WiFi.localIP().toString(), 10, 30);
  delay(1000); // delay to actually see the IP address

  // Start UDP
  udp.begin(udpPort);
  Serial.printf("UDP listening on port %d\n", udpPort);
}

void setup() {
  screenSetup();
  initWiFi();
  tft.fillScreen(TFT_BLACK);
  drawDashboard();
  delay(200);
}



void parseCarData(String data) {
  // Simple parsing - expecting: "speed,rpm,gear,fuel,throttle,brake"
  int commaIndex = 0;
  int lastIndex = 0;
  int fieldIndex = 0;

  for (int i = 0; i <= data.length(); i++) {
    if (data.charAt(i) == ',' || i == data.length()) {
      String field = data.substring(lastIndex, i);

      switch (fieldIndex) {
        case 0: carData.speed = field.toFloat(); break;
        case 1: carData.rpm = field.toInt(); break;
        case 2: carData.gear = field.toInt(); break;
        case 3: carData.fuel = field.toFloat(); break;
        case 4: carData.throttle = field.toFloat(); break;
        case 5: carData.brake = field.toFloat(); break;
      }

      lastIndex = i + 1;
      fieldIndex++;
    }
  }
}

void drawDashboard() {
  tft.fillScreen(TFT_BLACK);

  // Draw static elements
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  tft.drawString("SPEED", 20, 20);
  tft.drawString("km/h", 20, 70);
  tft.drawString("RPM", 160, 20);
  tft.drawString("GEAR", 260, 20);

  // Bottom info labels
  tft.setTextSize(1);
  tft.drawString("FUEL", 20, 140);
  tft.drawString("THROTTLE", 120, 140);
  tft.drawString("BRAKE", 220, 140);

  // Draw gauge backgrounds
  drawSpeedGauge();
  drawRPMBar();
}

void updateDashboard() {
  // Update speed
  tft.fillRect(20, 40, 100, 30, TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.drawString(String((int)carData.speed), 20, 40);

  // Update RPM
  tft.fillRect(160, 40, 80, 20, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.drawString(String(carData.rpm), 160, 40);

  // Update Gear
  tft.fillRect(260, 40, 40, 30, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(3);
  String gearStr = (carData.gear == 0) ? "N" : (carData.gear < 0) ? "R"
                                                                  : String(carData.gear);
  tft.drawString(gearStr, 260, 40);

  // Update fuel bar
  drawBar(20, 160, 80, 10, carData.fuel / 100.0, TFT_GREEN, TFT_RED);

  // Update throttle bar
  drawBar(120, 160, 80, 10, carData.throttle, TFT_GREEN, TFT_DARKGREY);

  // Update brake bar
  drawBar(220, 160, 80, 10, carData.brake, TFT_RED, TFT_DARKGREY);

  // Draw RPM bar graph
  drawRPMBar();
}

void drawBar(int x, int y, int width, int height, float percentage, uint16_t fillColor, uint16_t bgColor) {
  tft.drawRect(x, y, width, height, TFT_WHITE);
  tft.fillRect(x + 1, y + 1, width - 2, height - 2, bgColor);
  int fillWidth = (width - 2) * percentage;
  if (fillWidth > 0) {
    tft.fillRect(x + 1, y + 1, fillWidth, height - 2, fillColor);
  }
}

// TODO
void drawSpeedGauge() {
  // Simple arc for speed (0-200 km/h range)
  int centerX = 70;
  int centerY = 100;
  int radius = 40;

  // Draw speed arc background
  for (int i = 0; i <= 180; i += 5) {
    int x1 = centerX + cos((i - 90) * PI / 180) * radius;
    int y1 = centerY + sin((i - 90) * PI / 180) * radius;
    tft.drawPixel(x1, y1, TFT_DARKGREY);
  }
}

void drawRPMBar() {
  // Horizontal RPM bar (0-8000 RPM)
  int barX = 160;
  int barY = 70;
  int barWidth = 140;
  int barHeight = 15;

  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  tft.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, TFT_BLACK);

  // Fill based on RPM (max 8000)
  float rpmPercent = min(carData.rpm / 8000.0, 1.0);
  int fillWidth = (barWidth - 2) * rpmPercent;

  // Color coding: Green -> Yellow -> Red
  uint16_t rpmColor = TFT_GREEN;
  if (rpmPercent > 0.5) rpmColor = TFT_YELLOW;
  if (rpmPercent > 0.7) rpmColor = TFT_RED;

  if (fillWidth > 0) {
    tft.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, rpmColor);
  }
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    incomingPacket[len] = 0;
    parseCarData(incomingPacket);
    updateDashboard();
  }

  //delay(50);  // Update at ~20Hz
}
