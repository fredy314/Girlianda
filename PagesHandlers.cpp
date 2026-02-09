#include "PagesHandlers.h"
#include <WiFi.h>
#include "PagesHTML.h"

PagesHandlers::PagesHandlers(Garland& garlandA, Garland& garlandB) 
    : _garlandA(garlandA), _garlandB(garlandB) {
}

void PagesHandlers::initPagesHandlers(AsyncWebServer& webServer) {
    setupHomePageHandler(webServer);
    setupApiStatusHandler(webServer);
    setupApiSetHandler(webServer);
    
    // Favicon
    webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "image/svg+xml", HTML_CHRISTMAS_TREE_SVG);
    });
}

void PagesHandlers::setupHomePageHandler(AsyncWebServer& webServer) {
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", HTML_HOME);
    });
}

void PagesHandlers::setupApiStatusHandler(AsyncWebServer& webServer) {
    webServer.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"garlandA\":{";
        json += "\"mode\":" + String(_garlandA.getMode()) + ",";
        json += "\"speed\":" + String(_garlandA.getSpeed()) + ",";
        json += "\"brightness\":" + String(_garlandA.getBrightness());
        json += "},";
        json += "\"garlandB\":{";
        json += "\"mode\":" + String(_garlandB.getMode()) + ",";
        json += "\"speed\":" + String(_garlandB.getSpeed()) + ",";
        json += "\"brightness\":" + String(_garlandB.getBrightness());
        json += "},";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });
}

void PagesHandlers::setupApiSetHandler(AsyncWebServer& webServer) {
    webServer.on("/api/set", HTTP_GET, [this](AsyncWebServerRequest *request){
        String garland = "";
        Garland* targetGarland = nullptr;
        
        // Визначаємо цільову гірлянду
        if (request->hasParam("garland")) {
            garland = request->getParam("garland")->value();
            if (garland == "A") {
                targetGarland = &_garlandA;
            } else if (garland == "B") {
                targetGarland = &_garlandB;
            }
        }
        
        if (targetGarland == nullptr) {
            request->send(400, "application/json", "{\"error\":\"Invalid garland parameter\"}");
            return;
        }
        
        // Обробка параметрів
        if (request->hasParam("mode")) {
            int mode = request->getParam("mode")->value().toInt();
            targetGarland->setMode(mode);
        }
        if (request->hasParam("speed")) {
            int speed = request->getParam("speed")->value().toInt();
            targetGarland->setSpeed(speed);
        }
        if (request->hasParam("brightness")) {
            int brightness = request->getParam("brightness")->value().toInt();
            targetGarland->setBrightness(brightness);
        }
        
        // Повертаємо поточний стан
        String json = "{";
        json += "\"garland\":\"" + garland + "\",";
        json += "\"mode\":" + String(targetGarland->getMode()) + ",";
        json += "\"speed\":" + String(targetGarland->getSpeed()) + ",";
        json += "\"brightness\":" + String(targetGarland->getBrightness());
        json += "}";
        request->send(200, "application/json", json);
    });
}