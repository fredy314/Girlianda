#include <WiFi.h>
#include "esp_wifi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "secrets.h"
#include <WiFiManager.h>
#include <AuthenticationMiddleware.h>
#include "Garland.h"
#include "PagesHandlers.h"
// Pins
const int pinA = 10; 
const int pinB = 7;

// Globals
Garland garland(pinA, pinB);
AsyncWebServer server(80);
PagesHandlers pages(garland);
AuthenticationMiddleware authMiddleware;
WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Starting Girlianda ---");
  garland.begin();
  wifiManager.begin();
  authMiddleware.begin();
  server.addMiddleware(&authMiddleware);
  pages.initPagesHandlers(server);
  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  garland.tick();
  wifiManager.tick();
  ElegantOTA.loop();
}