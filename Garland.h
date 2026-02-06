#ifndef GARLAND_H
#define GARLAND_H

#include <Arduino.h>
#include <Preferences.h>
#include "driver/ledc.h"

class Garland {
public:
    enum Mode {
        MODE_OFF = 0,
        MODE_STEADY_ON = 1,
        MODE_ALTERNATING = 2, // Протифаза
        MODE_BREATHING_SYNC = 3, // Синхронне дихання
        MODE_CHAOS = 4, // Хаос
        MODE_FLICKER = 5 // Мерехтіння (Свічка)
    };

    Garland(int pinA, int pinB);
    void begin();
    void tick();

    void setMode(int mode);
    int getMode() const;

    void setSpeed(int speed); // 1-100, де 100 - найшвидше
    int getSpeed() const;

    void setBrightness(int brightness); // Для режиму постійного світіння
    int getBrightness() const;

private:
    int _pinA;
    int _pinB;
    int _mode;
    int _speed;
    int _manualBrightness; // Яскравість для режиму STEADY

    // Змінні для анімації
    int _currentBrightness;
    int _fadeAmount;
    unsigned long _lastUpdate;
    int _maxBrightness; // Ліміт яскравості (безпека)
    
    Preferences _prefs; // Об'єкт для збереження налаштувань

    // Внутрішні методи
    void _setupChannels(uint32_t hpoint0, uint32_t hpoint1);
    void _updateDuty(int val);
};

#endif
