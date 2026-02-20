#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H

#include <ESPWebMqttManager.h>
#include <ArduinoJson.h>
#include "Garland.h"
#include "secrets.h"

class MqttHelper {
public:
    static void setup(ESPWebMqttManager& mqtt, Garland& garlandA, Garland& garlandB) {
        mqtt.setHosts(mqttHosts, mqttHostCount);
        
        setupGarland(mqtt, "garland_a", "Гірлянда A", garlandA);
        setupGarland(mqtt, "garland_b", "Гірлянда B", garlandB);

        mqtt.begin();
    }

private:
    static void setupGarland(ESPWebMqttManager& mqtt, const char* id, const char* name, Garland& g) {
        String deviceId = mqtt.getDeviceId();
        mqtt.addHALight(id, name);
        mqtt.addHASelect((String(id) + "_mode").c_str(), (String(name) + " Режим").c_str(), "[\"Постійне\",\"Почергове\",\"Дихання\",\"Хаос\",\"Свічка\"]");
        mqtt.addHANumber((String(id) + "_speed").c_str(), (String(name) + " Швидкість").c_str(), 1, 100, "", "mdi:speedometer");

        // Команди
        String base = String("homeassistant/light/") + deviceId + "/" + id;
        mqtt.addCommand(base + "/set", [&g, id, name](String payload) {
            StaticJsonDocument<512> doc;
            if (!deserializeJson(doc, payload)) {
                if (doc.containsKey("state")) {
                    if (doc["state"] == "ON") g.setMode(1);
                    else { g.setBrightness(0); g.setMode(0); }
                }
                if (doc.containsKey("brightness")) g.setBrightness(doc["brightness"]);
            }
        });

        mqtt.addCommand(String("homeassistant/select/") + deviceId + "/" + id + "_mode/set", [&g](String payload) {
            if (payload == "Постійне") g.setMode(0);
            else if (payload == "Почергове") g.setMode(1);
            else if (payload == "Дихання") g.setMode(2);
            else if (payload == "Хаос") g.setMode(3);
            else if (payload == "Свічка") g.setMode(4);
        });

        // Сенсори (стан)
        mqtt.addSensor(base + "/state", [&g]() {
            StaticJsonDocument<256> doc;
            doc["state"] = (g.getMode() == 0 && g.getBrightness() == 0) ? "OFF" : "ON";
            doc["brightness"] = g.getBrightness();
            doc["color_mode"] = "brightness";
            String out; serializeJson(doc, out); return out;
        }, 5000);
    }
};

#endif
