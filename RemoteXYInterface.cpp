#include "RemoteXYInterface.h"
#include "secrets.h"

// Включення BLE бібліотеки ESP32 перед RemoteXY
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Конфігурація RemoteXY для BLE (після включення secrets.h)
#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME WIFI_HOSTNAME

#include <RemoteXY.h>

// Конфігурація інтерфейсу RemoteXY
// Згенеровано на сайті RemoteXY
#pragma pack(push, 1)
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] = // 52 bytes V19
  { 255,4,0,0,0,45,0,19,0,0,0,0,31,1,106,200,1,1,4,0,
  3,4,7,44,24,130,2,26,3,5,33,95,21,133,2,26,4,8,62,93,
  19,160,2,26,4,8,89,92,19,160,2,26 };

// Структура змінних RemoteXY
struct {
    // Вхідні змінні (з додатку)
    uint8_t Garland;     // from 0 to 2
    uint8_t Mode;        // from 0 to 5
    int8_t Speed;        // from -100 to 100
    int8_t Brightness;   // from -100 to 100

    // Інші змінні
    uint8_t connect_flag;  // =1 if wire connected, else =0
} RemoteXY;
#pragma pack(pop)

RemoteXYInterface::RemoteXYInterface(Garland& garlandA, Garland& garlandB)
    : _garlandA(garlandA), _garlandB(garlandB), _activeGarland(&garlandA),
      _lastGarlandSwitch(0), _lastModeSwitch(0), _lastSpeedSlider(50), _lastBrightnessSlider(50) {
}

void RemoteXYInterface::begin() {
    RemoteXY_Init();
    BLEDevice::getAdvertising()->stop(); // зупиняємо мовлення
    BLEDevice::init(WIFI_HOSTNAME);      // встановлюємо правильну назву
    BLEDevice::getAdvertising()->start(); // запускаємо знову
    // Ініціалізація початкових значень з активної гірлянди
    _activeGarland = &_garlandA;
    _syncFromGarland();
    
    Serial.println("RemoteXY BLE interface started");
    Serial.print("BLE Name: ");
    Serial.println(WIFI_HOSTNAME);
}

void RemoteXYInterface::tick() {
    RemoteXY_Handler();
    
    // Перевірка зміни активної гірлянди
    if (RemoteXY.Garland != _lastGarlandSwitch) {
        _lastGarlandSwitch = RemoteXY.Garland;
        _updateActiveGarland();
        _syncFromGarland(); // Синхронізувати значення з нової гірлянди
    }
    
    // Застосувати зміни якщо щось змінилось
    _applyChanges();
}

bool RemoteXYInterface::isConnected() const {
    return RemoteXY.connect_flag != 0;
}

void RemoteXYInterface::_updateActiveGarland() {
    if (RemoteXY.Garland == 0) {
        _activeGarland = &_garlandA;
        Serial.println("Switched to Garland A");
    } else {
        _activeGarland = &_garlandB;
        Serial.println("Switched to Garland B");
    }
}

void RemoteXYInterface::_syncFromGarland() {
    // Синхронізувати значення слайдерів з активної гірлянди
    RemoteXY.Mode = _activeGarland->getMode();
    // Конвертувати 0-100 в -100-100 для слайдерів
    RemoteXY.Speed = _activeGarland->getSpeed() * 2 - 100;
    RemoteXY.Brightness = _activeGarland->getBrightness() * 2 - 100;
    
    // Оновити попередні значення
    _lastModeSwitch = RemoteXY.Mode;
    _lastSpeedSlider = RemoteXY.Speed;
    _lastBrightnessSlider = RemoteXY.Brightness;
}

void RemoteXYInterface::_applyChanges() {
    bool changed = false;
    
    // Перевірка зміни режиму
    if (RemoteXY.Mode != _lastModeSwitch) {
        _activeGarland->setMode(RemoteXY.Mode, true);
        _lastModeSwitch = RemoteXY.Mode;
        changed = true;
        Serial.print("Mode changed to: ");
        Serial.println(RemoteXY.Mode);
    }
    
    // Перевірка зміни швидкості (конвертувати -100..100 в 0..100)
    if (RemoteXY.Speed != _lastSpeedSlider) {
        int speed = (RemoteXY.Speed + 100) / 2; // -100..100 -> 0..100
        _activeGarland->setSpeed(speed, true);
        _lastSpeedSlider = RemoteXY.Speed;
        changed = true;
        Serial.print("Speed changed to: ");
        Serial.println(speed);
    }
    
    // Перевірка зміни яскравості (конвертувати -100..100 в 0..100)
    if (RemoteXY.Brightness != _lastBrightnessSlider) {
        int brightness = (RemoteXY.Brightness + 100) / 2; // -100..100 -> 0..100
        _activeGarland->setBrightness(brightness, true);
        _lastBrightnessSlider = RemoteXY.Brightness;
        changed = true;
        Serial.print("Brightness changed to: ");
        Serial.println(brightness);
    }
}
