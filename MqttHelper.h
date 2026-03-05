/*
 * Girlianda project
 * Copyright (c) 2026 Fedir Vilhota <fredy31415@gmail.com>
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */
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
        
        // Перевіряємо чи активована Гірлянда Б
        Preferences prefs;
        prefs.begin("mqtt_helper", true);
        bool bEnabled = prefs.getBool("b_enabled", false);
        prefs.end();

        setupGarland(mqtt, "garland_a", "Гірлянда A", garlandA, true);
        setupGarland(mqtt, "garland_b", "Гірлянда B", garlandB, bEnabled);

        mqtt.begin();
    }

private:
    static void setupGarland(ESPWebMqttManager& mqtt, const char* id, const char* name, Garland& g, bool full) {
        String deviceId = mqtt.getDeviceId();
        String base = String("homeassistant/light/") + deviceId + "/" + id;
        const char* modesJson = "[\"Постійне\",\"Почергове\",\"Дихання\",\"Хаос\",\"Свічка\"]";

        // Використовуємо новий метод для підтримки яскравості та ефектів
        if (full) {
            mqtt.addHAJsonLight(id, name, modesJson);
            mqtt.addHANumber((String(id) + "_speed").c_str(), (String(name) + " Швидкість").c_str(), 1, 100, "", "mdi:speedometer");
        } else {
            mqtt.addHAJsonLight(id, name);
        }

        // Команда для світла (JSON схема з ефектами)
        mqtt.addCommand(base + "/set", [&mqtt, &g, id, name, full](String payload) {
            StaticJsonDocument<512> doc;
            if (!deserializeJson(doc, payload)) {
                if (doc.containsKey("state")) {
                    if (doc["state"] == "ON") {
                        if (g.getBrightness() == 0) g.setBrightness(255);
                        
                        // Якщо це Гірлянда Б і вона ще не активована повністю
                        if (String(id) == "garland_b" && !full) {
                            Preferences prefs;
                            prefs.begin("mqtt_helper", false);
                            prefs.putBool("b_enabled", true);
                            prefs.end();
                            mqtt.publishDiscovery(); 
                        }
                    } else {
                        g.setBrightness(0);
                    }
                }
                if (doc.containsKey("brightness")) {
                    g.setBrightness(doc["brightness"]);
                }
                if (doc.containsKey("effect")) {
                    String effect = doc["effect"];
                    if (effect == "Постійне") g.setMode(0);
                    else if (effect == "Почергове") g.setMode(1);
                    else if (effect == "Дихання") g.setMode(2);
                    else if (effect == "Хаос") g.setMode(3);
                    else if (effect == "Свічка") g.setMode(4);
                }
            }
        });

        if (full) {
            // Команда для швидкості
            mqtt.addCommand(String("homeassistant/number/") + deviceId + "/" + id + "_speed/set", [&g](String payload) {
                g.setSpeed(payload.toInt());
            });
        }

        // Сенсор стану (JSON для Light з ефектом)
        mqtt.addSensor(base + "/state", [&g, full]() {
            StaticJsonDocument<256> doc;
            doc["state"] = (g.getBrightness() == 0) ? "OFF" : "ON";
            doc["brightness"] = g.getBrightness();
            if (full) {
                int mode = g.getMode();
                switch(mode) {
                    case 0: doc["effect"] = "Постійне"; break;
                    case 1: doc["effect"] = "Почергове"; break;
                    case 2: doc["effect"] = "Дихання"; break;
                    case 3: doc["effect"] = "Хаос"; break;
                    case 4: doc["effect"] = "Свічка"; break;
                }
            }
            String out; serializeJson(doc, out); return out;
        }, 5000);

        if (full) {
            mqtt.addSensor(String("homeassistant/number/") + deviceId + "/" + id + "_speed/state", [&g]() {
                return String(g.getSpeed());
            }, 5000);
        }
    }
};

#endif
