#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "Garland.h"

class MqttManager {
public:
    MqttManager(Garland& garlandA, Garland& garlandB);
    
    void begin();
    void loop();
    
    // Публікація станів
    void publishGarlandStates();
    
private:
    Garland& _garlandA;
    Garland& _garlandB;
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    
    unsigned long _lastReconnectAttempt;
    unsigned long _lastStatePublish;
    bool _discoveryPublished;
    
    // MQTT налаштування
    static const int MQTT_PORT = 1883;
    static const char* MQTT_CLIENT_ID;
    static const char* DEVICE_NAME;
    static const char* DEVICE_ID;
    
    // Масив MQTT хостів та поточний індекс
    const char** _mqttHosts;
    int _mqttHostCount;
    int _currentHostIndex;
    
    // Топіки
    String _baseTopic;
    String _availabilityTopic;
    
    // Методи підключення
    bool reconnect();
    void publishDiscovery();
    void publishAvailability(bool online);
    
    // Callback для вхідних повідомлень
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(String topic, String payload);
    
    // Допоміжні методи для створення конфігурацій HomeAssistant
    void publishSelectConfig(const char* garland, const char* name);
    void publishNumberConfig(const char* garland, const char* parameter, const char* name, int min, int max, const char* unit, const char* icon);
    
    // Методи для публікації станів окремих гірлянд
    void publishGarlandState(const char* garland, Garland& garlandObj);
    
    // Допоміжний метод для отримання назви режиму
    const char* getModeName(int mode);
};

#endif // MQTT_MANAGER_H
