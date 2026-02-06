#include <WiFi.h>
#include "esp_wifi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Garland.h"
#include "PagesHandlers.h"

// --- Configuration ---
const char* ssid = "HomeF";
const char* ssidExt = "HomeF_EXT";
const char* password = "21122112";
/*
// Static IP settings
// Примітка: переконайтеся, що ця IP вільна і відповідає вашій підмережі
IPAddress staticIP(192, 168, 0, 241);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
*/
// Pins
const int pinA = 10; 
const int pinB = 7;

// Globals
// Globals
Garland garland(pinA, pinB);
AsyncWebServer server(80);
PagesHandlers pages(garland);
unsigned long wifiConnectStart = 0;
int wifiConnectStage = 0; // 0: Stopped, 1: Primary, 2: Secondary, 3: Scanning
int wifiPowerIndex = 0; // 0..3
bool isScanning = false;

wifi_power_t powerSteps[] = {
  WIFI_POWER_8_5dBm,  // Мінімальна (найбезпечніша для USB)
  WIFI_POWER_11dBm,   // Трохи сильніша
  WIFI_POWER_13dBm,   // Середня
  WIFI_POWER_15dBm    // Максимальна "безпечна" для вашої збірки
};

// Non-blocking connection initiator
void startWiFiConnection(const char* targetSsid, const char* password, wifi_power_t power) {
  Serial.print("Connecting to: ");
  Serial.print(targetSsid);
  Serial.print(" [Power Index: ");
  Serial.print(wifiPowerIndex);
  Serial.println("]");

  // Stop scan if running (clean up)
  if (isScanning) {
    WiFi.scanDelete();
    isScanning = false;
  }

  // 1. Повністю скидаємо WiFi стек
  WiFi.disconnect(true);  // Disconnect and turn OFF radio
  delay(500);             // Give it a moment to shut down
  
  WiFi.mode(WIFI_STA);    // Turn ON radio in STA mode
  delay(100);
  
  // 2. Застосовуємо наші "пробивні" налаштування
  WiFi.setSleep(false);
  WiFi.setTxPower(power);
  // sp_wifi_set_protocol removed to allow auto-negotiation

  // 3. Запускаємо підключення
  WiFi.begin(targetSsid, password);
  
  // Reset timer
  wifiConnectStart = millis();
}

void setup() {
  // 0. Init Serial with retry
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n\n--- Starting Girlianda Debug Mode ---");

  // 1. Setup Garland Logic
  garland.begin();
    
  // 2. Setup WiFi - Start with Async Scan
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.println("Starting background WiFi scan...");
  WiFi.scanNetworks(true); // Async scan
  isScanning = true;
  wifiConnectStage = 3; // Scanning state

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
      Serial.println("Restarting WiFi scan & connect...");
      WiFi.scanDelete();
      WiFi.disconnect();
      WiFi.scanNetworks(true);
      isScanning = true;
      wifiConnectStage = 3;
      
    } else if (input == "ap") {
      Serial.println("Forcing AP Mode...");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Girlianda-AP", "12345678");
      Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
      wifiConnectStage = 0; // Stop trying
      isScanning = false;
    } else if (input == "status") {
      Serial.println("--- Status ---");
      Serial.print("Mode: "); Serial.println(garland.getMode());
      Serial.print("Speed: "); Serial.println(garland.getSpeed());
      Serial.print("Brightness: "); Serial.println(garland.getBrightness());
      Serial.print("WiFi Mode: "); Serial.println(WiFi.getMode() == WIFI_AP ? "AP" : "STA");
      Serial.print("STA IP: "); Serial.println(WiFi.localIP());
      Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
      Serial.println("--------------");
    } else if (input == "scan") {
      Serial.println("Manual scan triggered...");
      int n = WiFi.scanNetworks();
      if (n == 0) {
          Serial.println("No networks found.");
      } else {
          Serial.printf("Found %d networks:\n", n);
          for (int i = 0; i < n; ++i) {
              Serial.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
              delay(10);
          }
      }
      Serial.println("-----------------------");
    } else {
      Serial.println("Unknown command. Use m=X, s=X, b=X, wifi, ap, status, scan");
    }
  }
}

void loop() {
  // Main Logic Tick
  garland.tick();

  // Serial Control
  handleSerial();

  // WiFi Logic State Machine
  if (wifiConnectStage == 3) {
      // 3: Scanning
      int n = WiFi.scanComplete();
      if (n >= 0) {
          Serial.println("Scan complete.");
          isScanning = false;
          
          String bestSSID = "";
          int bestRSSI = -1000;
          
          for (int i = 0; i < n; ++i) {
              String currentSSID = WiFi.SSID(i);
              int currentRSSI = WiFi.RSSI(i);
              
              if (currentSSID == ssid || currentSSID == ssidExt) {
                  Serial.printf("Found target: %s (%d dBm)\n", currentSSID.c_str(), currentRSSI);
                  if (currentRSSI > bestRSSI) {
                      bestRSSI = currentRSSI;
                      bestSSID = currentSSID;
                  }
              }
          }
          WiFi.scanDelete(); // Clean up RAM
          
          wifiPowerIndex = 0; // Start with lowest power
          if (bestSSID != "") {
              Serial.print("Best network: "); Serial.println(bestSSID);
              // Decide stage based on which SSID it is
              if (bestSSID == ssid) {
                  startWiFiConnection(ssid, password, powerSteps[wifiPowerIndex]);
                  wifiConnectStage = 1; // Trying Primary
              } else {
                  startWiFiConnection(ssidExt, password, powerSteps[wifiPowerIndex]);
                  wifiConnectStage = 2; // Trying Secondary
              }
          } else {
              Serial.println("Targets not found. Defaulting to Primary (Power 0).");
              startWiFiConnection(ssid, password, powerSteps[wifiPowerIndex]);
              wifiConnectStage = 1;
          }
      } else if (n == -2) {
           Serial.println("Scan failed. Defaulting to Primary (Power 0).");
           wifiPowerIndex = 0;
           startWiFiConnection(ssid, password, powerSteps[wifiPowerIndex]);
           wifiConnectStage = 1;
           isScanning = false;
      }
  } 
  else if (wifiConnectStage > 0) {
    // Connection Attempt States
    if (WiFi.status() == WL_CONNECTED) {
       Serial.println("WiFi Connected!");
       Serial.print("Station IP: "); Serial.println(WiFi.localIP());
       wifiConnectStage = 0; // Done
    } 
    // Check for timeout (10 seconds)
    else if (millis() - wifiConnectStart > 10000) {
       if (wifiConnectStage == 1) {
          // Timeout Primary
          Serial.println("Primary failed attempt.");
          wifiPowerIndex++;
          
          if (wifiPowerIndex < 4) {
             // Retry with next power level
             Serial.print("Retrying Primary with Power Level "); Serial.println(wifiPowerIndex);
             startWiFiConnection(ssid, password, powerSteps[wifiPowerIndex]);
          } else {
             // All powers failed for Primary -> Switch to Secondary
             Serial.println("Primary exhausted. Switching to Secondary.");
             wifiPowerIndex = 0; // Reset power for secondary
             startWiFiConnection(ssidExt, password, powerSteps[wifiPowerIndex]);
             wifiConnectStage = 2;
          }
       } else if (wifiConnectStage == 2) {
          // Timeout Secondary
          Serial.println("Secondary failed attempt.");
          wifiPowerIndex++;

          if (wifiPowerIndex < 4) {
              // Retry with next power level
              Serial.print("Retrying Secondary with Power Level "); Serial.println(wifiPowerIndex);
              startWiFiConnection(ssidExt, password, powerSteps[wifiPowerIndex]);
          } else {
              // All powers failed for Secondary -> AP
              Serial.println("All attempts failed. Starting Access Point.");
              Serial.println(WiFi.status());
              WiFi.disconnect();
              WiFi.mode(WIFI_AP);
              WiFi.softAP("Girlianda-AP", "12345678");
              Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
              wifiConnectStage = 0; // Done
          }
       }
    }
  }
}