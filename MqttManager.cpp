#include "MqttManager.h"
#include "secrets.h"

// Статичні константи
const char* MqttManager::MQTT_CLIENT_ID = "esp32_girlianda";
const char* MqttManager::DEVICE_NAME = "Гірлянда";
const char* MqttManager::DEVICE_ID = "girlianda_01";

// Статичний вказівник для callback
static MqttManager* instance = nullptr;

MqttManager::MqttManager(Garland& garlandA, Garland& garlandB)
    : _garlandA(garlandA), _garlandB(garlandB), _mqttClient(_wifiClient),
      _lastReconnectAttempt(0), _lastStatePublish(0),
      _discoveryPublished(false),
      _lastActiveModeA(1), _lastActiveModeB(1),
      _mqttHosts(mqttHosts), _mqttHostCount(mqttHostCount), _currentHostIndex(0),
      _lastSentModeA(-1), _lastSentBrightnessA(-1), _lastSentSpeedA(-1),
      _lastSentModeB(-1), _lastSentBrightnessB(-1), _lastSentSpeedB(-1),
      _lastPeriodicPublish(0)
{
    instance = this;
    _baseTopic = String("homeassistant");
    _availabilityTopic = String("homeassistant/") + DEVICE_ID + "/availability";
}

void MqttManager::begin() {
    // Встановлюємо перший хост з масиву
    _mqttClient.setServer(_mqttHosts[_currentHostIndex], MQTT_PORT);
    _mqttClient.setCallback(messageCallback);
    _mqttClient.setBufferSize(2048); // Збільшуємо буфер для великих повідомлень (JSON)
    Serial.printf("MQTT: Using host %s\n", _mqttHosts[_currentHostIndex]);
}

void MqttManager::loop() {
    if (!_mqttClient.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            if (reconnect()) {
                _lastReconnectAttempt = 0;
            }
        }
    } else {
        _mqttClient.loop();
        
        // Публікуємо стани гірлянд кожні 5 секунд
        unsigned long now = millis();
        if (now - _lastStatePublish > 5000) {
            _lastStatePublish = now;
            publishGarlandStates();
        }
        
        // Періодична публікація availability та discovery кожні 10 хвилин
        if (now - _lastPeriodicPublish > 600000) { // 600000 мс = 10 хвилин
            _lastPeriodicPublish = now;
            Serial.println("MQTT: Periodic publish of availability and discovery");
            publishAvailability(true);
            publishDiscovery();
            _lastSentModeA=-1;
            _lastSentModeB=-1;
            _lastSentBrightnessA=-1;
            _lastSentBrightnessB=-1;
            _lastSentSpeedA=-1;
            _lastSentSpeedB=-1;
            publishGarlandStates();
        }
    }
}

bool MqttManager::reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    Serial.printf("Connecting to MQTT %s...", _mqttHosts[_currentHostIndex]);
    
    if (_mqttClient.connect(MQTT_CLIENT_ID, _availabilityTopic.c_str(), 0, true, "offline")) {
        Serial.println(" connected");
        
        publishAvailability(true);
        
        // Публікуємо конфігурацію для HomeAssistant Discovery
        if (!_discoveryPublished) {
            publishDiscovery();
            _discoveryPublished = true;
        }
        
        // Підписуємось на команди для гірлянди A
        _mqttClient.subscribe("homeassistant/light/girlianda/garland_a/set");
        _mqttClient.subscribe("homeassistant/select/girlianda/garland_a_mode/set");
        _mqttClient.subscribe("homeassistant/number/girlianda/garland_a_speed/set");
        
        // Підписуємось на команди для гірлянди B
        _mqttClient.subscribe("homeassistant/light/girlianda/garland_b/set");
        _mqttClient.subscribe("homeassistant/select/girlianda/garland_b_mode/set");
        _mqttClient.subscribe("homeassistant/number/girlianda/garland_b_speed/set");
        
        return true;
    }
    
    Serial.print(" failed, rc=");
    Serial.println(_mqttClient.state());
    
    // Якщо не вдалося підключитися, пробуємо наступний хост
    if (_mqttHostCount > 1) {
        _currentHostIndex = (_currentHostIndex + 1) % _mqttHostCount;
        Serial.printf("MQTT: Switching to next host: %s\n", _mqttHosts[_currentHostIndex]);
        _mqttClient.setServer(_mqttHosts[_currentHostIndex], MQTT_PORT);
    }
    
    return false;
}

void MqttManager::publishAvailability(bool online) {
    _mqttClient.publish(_availabilityTopic.c_str(), online ? "online" : "offline", true);
}

void MqttManager::publishDiscovery() {
    // Публікуємо конфігурацію для гірлянди A
    publishLightConfig("garland_a", "Гірлянда A");
    publishSelectConfig("garland_a", "Гірлянда A Режим");
    publishNumberConfig("garland_a", "speed", "Гірлянда A Швидкість", 1, 100, "", "mdi:speedometer");
    
    // Публікуємо конфігурацію для гірлянди B
    publishLightConfig("garland_b", "Гірлянда B");
    publishSelectConfig("garland_b", "Гірлянда B Режим");
    publishNumberConfig("garland_b", "speed", "Гірлянда B Швидкість", 1, 100, "", "mdi:speedometer");
}

void MqttManager::publishLightConfig(const char* garland, const char* name) {
    String configTopic = String("homeassistant/light/girlianda/") + garland + "/config";
    String stateTopic = String("homeassistant/light/girlianda/") + garland + "/state";
    String cmdTopic = String("homeassistant/light/girlianda/") + garland + "/set";
    
    String config = "{";
    config += "\"name\":\"" + String(name) + "\",";
    config += "\"unique_id\":\"" + String(DEVICE_ID) + "_" + garland + "_light\",";
    config += "\"state_topic\":\"" + stateTopic + "\",";
    config += "\"command_topic\":\"" + cmdTopic + "\",";
    config += "\"brightness\":true,";
    config += "\"brightness_scale\":255,";
    config += "\"schema\":\"json\",";
    config += "\"color_mode\":true,";
    config += "\"supported_color_modes\":[\"brightness\"],";
    config += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    config += "\"device\":{";
    config += "\"identifiers\":[\"" + String(DEVICE_ID) + "\"],";
    config += "\"name\":\"" + String(DEVICE_NAME) + "\",";
    config += "\"manufacturer\":\"Custom\",";
    config += "\"model\":\"ESP32 Garland Controller\"";
    config += "}}";
    
    _mqttClient.publish(configTopic.c_str(), config.c_str(), true);
}

void MqttManager::publishSelectConfig(const char* garland, const char* name) {
    String configTopic = String("homeassistant/select/girlianda/") + garland + "_mode/config";
    String stateTopic = String("homeassistant/select/girlianda/") + garland + "_mode/state";
    String cmdTopic = String("homeassistant/select/girlianda/") + garland + "_mode/set";
    
    String config = "{";
    config += "\"name\":\"" + String(name) + "\",";
    config += "\"unique_id\":\"" + String(DEVICE_ID) + "_" + garland + "_mode\",";
    config += "\"state_topic\":\"" + stateTopic + "\",";
    config += "\"command_topic\":\"" + cmdTopic + "\",";
    config += "\"options\":[\"Постійне\",\"Почергове\",\"Дихання\",\"Хаос\",\"Свічка\"],";
    config += "\"icon\":\"mdi:lightbulb-group\",";
    config += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    config += "\"device\":{";
    config += "\"identifiers\":[\"" + String(DEVICE_ID) + "\"],";
    config += "\"name\":\"" + String(DEVICE_NAME) + "\",";
    config += "\"manufacturer\":\"Custom\",";
    config += "\"model\":\"ESP32 Garland Controller\"";
    config += "}}";
    
    _mqttClient.publish(configTopic.c_str(), config.c_str(), true);
}

void MqttManager::publishNumberConfig(const char* garland, const char* parameter, const char* name, int min, int max, const char* unit, const char* icon) {
    String configTopic = String("homeassistant/number/girlianda/") + garland + "_" + parameter + "/config";
    String stateTopic = String("homeassistant/number/girlianda/") + garland + "_" + parameter + "/state";
    String cmdTopic = String("homeassistant/number/girlianda/") + garland + "_" + parameter + "/set";
    
    String config = "{";
    config += "\"name\":\"" + String(name) + "\",";
    config += "\"unique_id\":\"" + String(DEVICE_ID) + "_" + garland + "_" + parameter + "\",";
    config += "\"state_topic\":\"" + stateTopic + "\",";
    config += "\"command_topic\":\"" + cmdTopic + "\",";
    config += "\"min\":" + String(min) + ",";
    config += "\"max\":" + String(max) + ",";
    if (strlen(unit) > 0) config += "\"unit_of_measurement\":\"" + String(unit) + "\",";
    if (strlen(icon) > 0) config += "\"icon\":\"" + String(icon) + "\",";
    config += "\"availability_topic\":\"" + _availabilityTopic + "\",";
    config += "\"device\":{";
    config += "\"identifiers\":[\"" + String(DEVICE_ID) + "\"],";
    config += "\"name\":\"" + String(DEVICE_NAME) + "\"";
    config += "}}";
    
    _mqttClient.publish(configTopic.c_str(), config.c_str(), true);
}

void MqttManager::publishGarlandStates() {
    publishGarlandState("garland_a", _garlandA, _lastSentModeA, _lastSentBrightnessA, _lastSentSpeedA);
    publishGarlandState("garland_b", _garlandB, _lastSentModeB, _lastSentBrightnessB, _lastSentSpeedB);
}

void MqttManager::publishGarlandState(const char* garland, Garland& garlandObj, int& lastMode, int& lastBrightness, int& lastSpeed, bool force) {
    int currentMode = garlandObj.getMode();
    int currentBrightness = garlandObj.getBrightness();
    int currentSpeed = garlandObj.getSpeed();
    
    if (!force && currentMode == lastMode && currentBrightness == lastBrightness && currentSpeed == lastSpeed) {
        return; // Змін немає, нічого не публікуємо
    }

    // Публікуємо стан світла (schema: json)
    String lightStateTopic = String("homeassistant/light/girlianda/") + garland + "/state";
    StaticJsonDocument<256> doc;
    doc["state"] = (currentMode == 0 && currentBrightness == 0) ? "OFF" : "ON";
    // Оскільки в Garland яскравість використовується тільки для CONSTANT режиму, 
    // ми публікуємо її як стан яскравості лампи
    doc["brightness"] = currentBrightness;
    doc["color_mode"] = "brightness";
    
    String output;
    serializeJson(doc, output);
    _mqttClient.publish(lightStateTopic.c_str(), output.c_str());

    // Публікуємо режим
    String modeStateTopic = String("homeassistant/select/girlianda/") + garland + "_mode/state";
    _mqttClient.publish(modeStateTopic.c_str(), getModeName(currentMode));
    
    // Публікуємо швидкість
    String speedStateTopic = String("homeassistant/number/girlianda/") + garland + "_speed/state";
    _mqttClient.publish(speedStateTopic.c_str(), String(currentSpeed).c_str());
    
    // Оновлюємо останні надіслані значення
    lastMode = currentMode;
    lastBrightness = currentBrightness;
    lastSpeed = currentSpeed;
}

const char* MqttManager::getModeName(int mode) {
    switch(mode) {
        case 0: return "Постійне";
        case 1: return "Почергове";
        case 2: return "Дихання";
        case 3: return "Хаос";
        case 4: return "Свічка";
        default: return "Постійне";
    }
}

void MqttManager::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        String topicStr = String(topic);
        String payloadStr = "";
        for (unsigned int i = 0; i < length; i++) {
            payloadStr += (char)payload[i];
        }
        instance->handleMessage(topicStr, payloadStr);
    }
}

void MqttManager::handleMessage(String topic, String payload) {
    Serial.print(F("MQTT message: "));
    Serial.print(topic);
    Serial.print(F(" = "));
    Serial.println(payload);
    
    // Обробка команд для гірлянди A (Light JSON)
    if (topic == "homeassistant/light/girlianda/garland_a/set") {
        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            if (doc.containsKey("state")) {
                String state = doc["state"];
                if (state == "ON") {
                    _garlandA.setMode(_lastActiveModeA);
                } else if (state == "OFF") {
                    if (_garlandA.getMode() > 0) _lastActiveModeA = _garlandA.getMode();
                    _garlandA.setBrightness(0);
                    _garlandA.setMode(0);
                }
            }
            if (doc.containsKey("brightness")) {
                int brightness = doc["brightness"];
                _garlandA.setBrightness(brightness);
                if (brightness > 0 && _garlandA.getMode() == 0) {
                     _garlandA.setMode(_lastActiveModeA);
                }
            }
        }
        publishGarlandState("garland_a", _garlandA, _lastSentModeA, _lastSentBrightnessA, _lastSentSpeedA);
        return;
    }
    
    // Обробка режимів для гірлянди A
    if (topic == "homeassistant/select/girlianda/garland_a_mode/set") {
        int mode = 0;
        if (payload == "Постійне") mode = 0;
        else if (payload == "Почергове") mode = 1;
        else if (payload == "Дихання") mode = 2;
        else if (payload == "Хаос") mode = 3;
        else if (payload == "Свічка") mode = 4;
        
        _lastActiveModeA = mode;
        _garlandA.setMode(mode);
        publishGarlandState("garland_a", _garlandA, _lastSentModeA, _lastSentBrightnessA, _lastSentSpeedA);
        return;
    }
    
    // Обробка швидкості для гірлянди A
    if (topic == "homeassistant/number/girlianda/garland_a_speed/set") {
        int speed = payload.toInt();
        _garlandA.setSpeed(speed);
        publishGarlandState("garland_a", _garlandA, _lastSentModeA, _lastSentBrightnessA, _lastSentSpeedA);
        return;
    }
    
    // Обробка команд для гірлянди B (Light JSON)
    if (topic == "homeassistant/light/girlianda/garland_b/set") {
        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            if (doc.containsKey("state")) {
                String state = doc["state"];
                if (state == "ON") {
                    _garlandB.setMode(_lastActiveModeB);
                } else if (state == "OFF") {
                    if (_garlandB.getMode() > 0) _lastActiveModeB = _garlandB.getMode();
                    _garlandB.setBrightness(0);
                    _garlandB.setMode(0);
                }
            }
            if (doc.containsKey("brightness")) {
                int brightness = doc["brightness"];
                _garlandB.setBrightness(brightness);
                if (brightness > 0 && _garlandB.getMode() == 0) {
                     _garlandB.setMode(_lastActiveModeB);
                }
            }
        }
        publishGarlandState("garland_b", _garlandB, _lastSentModeB, _lastSentBrightnessB, _lastSentSpeedB);
        return;
    }
    
    // Обробка режимів для гірлянди B
    if (topic == "homeassistant/select/girlianda/garland_b_mode/set") {
        int mode = 0;
        if (payload == "Постійне") mode = 0;
        else if (payload == "Почергове") mode = 1;
        else if (payload == "Дихання") mode = 2;
        else if (payload == "Хаос") mode = 3;
        else if (payload == "Свічка") mode = 4;
        
        _lastActiveModeB = mode;
        _garlandB.setMode(mode);
        publishGarlandState("garland_b", _garlandB, _lastSentModeB, _lastSentBrightnessB, _lastSentSpeedB);
        return;
    }
    
    // Обробка швидкості для гірлянди B
    if (topic == "homeassistant/number/girlianda/garland_b_speed/set") {
        int speed = payload.toInt();
        _garlandB.setSpeed(speed);
        publishGarlandState("garland_b", _garlandB, _lastSentModeB, _lastSentBrightnessB, _lastSentSpeedB);
        return;
    }
}
