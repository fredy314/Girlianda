/*
 * Girlianda project
 * Copyright (c) 2026 Fredy
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */
#include <WiFi.h>
#include "esp_wifi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "secrets.h"
#include <WiFiManager.h>
#include <AuthenticationMiddleware.h>
#include <ESPWebMqttManager.h>
#include <ArduinoJson.h>
#include "Garland.h"
#include "PagesHandlers.h"

#include "MqttHelper.h"

// Pins
const int pinA1 = 0; 
const int pinA2 = 1;
const int pinB1 = 3;
const int pinB2 = 4;

// Globals
Garland garlandA(pinA1, pinA2, 0, 1, 0, "garlandA");  // Канали 0,1, Таймер 0
Garland garlandB(pinB1, pinB2, 2, 3, 1, "garlandB");  // Канали 2,3, Таймер 1
AsyncWebServer server(80);
PagesHandlers pages(garlandA, garlandB);
WiFiManager wifiManager;
AuthenticationMiddleware authMiddleware;
ESPWebMqttManager mqttManager("girlianda", "Гірлянда");

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Starting Girlianda (2 channels) ---");
  
  
  garlandA.begin();
  garlandB.begin();
  wifiManager.begin();
  authMiddleware.begin();
  // Запуск MQTT
  MqttHelper::setup(mqttManager, garlandA, garlandB);
  
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
  mqttManager.loop();
  ElegantOTA.loop();
}
