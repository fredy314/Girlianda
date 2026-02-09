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
const int pinA1 = 10; 
const int pinA2 = 7;
const int pinB1 = 6;
const int pinB2 = 5;

// Globals
Garland garlandA(pinA1, pinA2, 0, 1, 0, "garlandA");  // Канали 0,1, Таймер 0
Garland garlandB(pinB1, pinB2, 2, 3, 1, "garlandB");  // Канали 2,3, Таймер 1
AsyncWebServer server(80);
PagesHandlers pages(garlandA, garlandB);
AuthenticationMiddleware authMiddleware;
WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Starting Girlianda (2 channels) ---");
  garlandA.begin();
  garlandB.begin();
  wifiManager.begin();
  authMiddleware.begin();
  server.addMiddleware(&authMiddleware);
  pages.initPagesHandlers(server);
  ElegantOTA.begin(&server);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  garlandA.tick();
  garlandB.tick();
  wifiManager.tick();
  ElegantOTA.loop();
}
