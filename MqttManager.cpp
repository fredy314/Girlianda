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
      _mqttHosts(mqttHosts), _mqttHostCount(mqttHostCount), _currentHostIndex(0)
{
    instance = this;
    _baseTopic = String("homeassistant");
    _availabilityTopic = String("homeassistant/") + DEVICE_ID + "/availability";
}

void MqttManager::begin() {
    // Встановлюємо перший хост з масиву
    _mqttClient.setServer(_mqttHosts[_currentHostIndex], MQTT_PORT);
    _mqttClient.setCallback(messageCallback);
    _mqttClient.setBufferSize(1024); // Збільшуємо буфер для великих повідомлень
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
        _mqttClient.subscribe("homeassistant/select/girlianda/garland_a_mode/set");
        _mqttClient.subscribe("homeassistant/number/girlianda/garland_a_brightness/set");
        _mqttClient.subscribe("homeassistant/number/girlianda/garland_a_speed/set");
        
        // Підписуємось на команди для гірлянди B
        _mqttClient.subscribe("homeassistant/select/girlianda/garland_b_mode/set");
        _mqttClient.subscribe("homeassistant/number/girlianda/garland_b_brightness/set");
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
    publishSelectConfig("garland_a", "Гірлянда A Режим");
    publishNumberConfig("garland_a", "brightness", "Гірлянда A Яскравість", 0, 255, "", "mdi:brightness-6");
    publishNumberConfig("garland_a", "speed", "Гірлянда A Швидкість", 1, 100, "", "mdi:speedometer");
    
    // Публікуємо конфігурацію для гірлянди B
    publishSelectConfig("garland_b", "Гірлянда B Режим");
    publishNumberConfig("garland_b", "brightness", "Гірлянда B Яскравість", 0, 255, "", "mdi:brightness-6");
    publishNumberConfig("garland_b", "speed", "Гірлянда B Швидкість", 1, 100, "", "mdi:speedometer");
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
    publishGarlandState("garland_a", _garlandA);
    publishGarlandState("garland_b", _garlandB);
}

void MqttManager::publishGarlandState(const char* garland, Garland& garlandObj) {
    // Публікуємо режим
    String modeStateTopic = String("homeassistant/select/girlianda/") + garland + "_mode/state";
    _mqttClient.publish(modeStateTopic.c_str(), getModeName(garlandObj.getMode()));
    
    // Публікуємо яскравість
    String brightnessStateTopic = String("homeassistant/number/girlianda/") + garland + "_brightness/state";
    _mqttClient.publish(brightnessStateTopic.c_str(), String(garlandObj.getBrightness()).c_str());
    
    // Публікуємо швидкість
    String speedStateTopic = String("homeassistant/number/girlianda/") + garland + "_speed/state";
    _mqttClient.publish(speedStateTopic.c_str(), String(garlandObj.getSpeed()).c_str());
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
    
    // Обробка команд для гірлянди A
    if (topic == "homeassistant/select/girlianda/garland_a_mode/set") {
        int mode = 0;
        if (payload == "Постійне") mode = 0;
        else if (payload == "Почергове") mode = 1;
        else if (payload == "Дихання") mode = 2;
        else if (payload == "Хаос") mode = 3;
        else if (payload == "Свічка") mode = 4;
        _garlandA.setMode(mode);
        publishGarlandState("garland_a", _garlandA);
    }
    else if (topic == "homeassistant/number/girlianda/garland_a_brightness/set") {
        int brightness = payload.toInt();
        _garlandA.setBrightness(brightness);
        publishGarlandState("garland_a", _garlandA);
    }
    else if (topic == "homeassistant/number/girlianda/garland_a_speed/set") {
        int speed = payload.toInt();
        _garlandA.setSpeed(speed);
        publishGarlandState("garland_a", _garlandA);
    }
    
    // Обробка команд для гірлянди B
    else if (topic == "homeassistant/select/girlianda/garland_b_mode/set") {
        int mode = 0;
        if (payload == "Постійне") mode = 0;
        else if (payload == "Почергове") mode = 1;
        else if (payload == "Дихання") mode = 2;
        else if (payload == "Хаос") mode = 3;
        else if (payload == "Свічка") mode = 4;
        _garlandB.setMode(mode);
        publishGarlandState("garland_b", _garlandB);
    }
    else if (topic == "homeassistant/number/girlianda/garland_b_brightness/set") {
        int brightness = payload.toInt();
        _garlandB.setBrightness(brightness);
        publishGarlandState("garland_b", _garlandB);
    }
    else if (topic == "homeassistant/number/girlianda/garland_b_speed/set") {
        int speed = payload.toInt();
        _garlandB.setSpeed(speed);
        publishGarlandState("garland_b", _garlandB);
    }
}
