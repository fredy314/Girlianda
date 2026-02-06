#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Garland.h"
#include "PagesHandlers.h"

// --- Configuration ---
const char* ssid = "HomeF";
const char* password = "21122112";

// Static IP settings
// Примітка: переконайтеся, що ця IP вільна і відповідає вашій підмережі
IPAddress staticIP(192, 168, 0, 241);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);

// Pins
const int pinA = 10; 
const int pinB = 7;

// Globals
Garland garland(pinA, pinB);
AsyncWebServer server(80);
PagesHandlers pages(garland);

void setup() {
  // 0. Init Serial with retry
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n\n--- Starting Girlianda Debug Mode ---");

  // 1. Setup Garland Logic
  garland.begin();
  
  // Visual Feedback: Fast flash = Booting
  garland.setBrightness(255);
  garland.setMode(Garland::MODE_STEADY_ON);
  delay(200); 
  garland.setMode(Garland::MODE_OFF);
  delay(200);
  garland.setMode(Garland::MODE_ALTERNATING);
  
  // 2. Setup WiFi with Timeout and AP Fallback
  // Try static config first
  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS)) {
    Serial.println("Static IP Config Failed!");
  }
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  unsigned long startAttempt = millis();
  bool connected = false;

  // Try connecting for 15 seconds
  while (millis() - startAttempt < 15000) {
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    
    // Critical: Keep animation running!
    garland.tick(); 
    
    // Serial feedback every 500ms
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
      lastPrint = millis();
      Serial.print(".");
    }
    delay(10); 
  }

  Serial.println();
  
  if (connected) {
    Serial.println("WiFi Connected!");
    Serial.print("Station IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connection Failed! Starting Access Point.");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Girlianda-AP", "12345678"); // Pass: 12345678
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  }

  // 3. Setup Web Server
  pages.initPagesHandlers(server);
  server.begin();
  Serial.println("Web server started");
}

void handleSerial() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    // Commands:
    // m=0..5 (Mode)
    // s=0..100 (Speed)
    // b=0..255 (Brightness)
    // wifi (Print WiFi Status)
    // ap (Force AP Mode)
    
    if (input.startsWith("m=")) {
      int val = input.substring(2).toInt();
      garland.setMode(val);
      Serial.print("Mode set to: "); Serial.println(val);
    } else if (input.startsWith("s=")) {
      int val = input.substring(2).toInt();
      garland.setSpeed(val);
      Serial.print("Speed set to: "); Serial.println(val);
    } else if (input.startsWith("b=")) {
      int val = input.substring(2).toInt();
      garland.setBrightness(val);
      Serial.print("Brightness set to: "); Serial.println(val);
    } else if (input == "wifi") {
      Serial.print("WiFi Mode: "); Serial.println(WiFi.getMode());
      Serial.print("STA IP: "); Serial.println(WiFi.localIP());
      Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
    } else if (input == "ap") {
      Serial.println("Forcing AP Mode...");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Girlianda-AP", "12345678");
      Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
    } else {
      Serial.println("Unknown command. Use m=X, s=X, b=X, wifi, ap");
    }
  }
}

void loop() {
  // Main Logic Tick
  garland.tick();

  // Serial Control
  handleSerial();

  /*// Logging Heartbeat to prove code is running and show IP
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    lastLog = millis();
    Serial.print("Status: Running. Mode: ");
    Serial.print(garland.getMode());
    Serial.print(" IP: ");
    if (WiFi.getMode() == WIFI_AP) {
       Serial.print(WiFi.softAPIP());
       Serial.print(" (AP)");
    } else {
       Serial.print(WiFi.localIP());
    }
    Serial.println();
  }*/
}