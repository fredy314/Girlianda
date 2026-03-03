/*
 * Girlianda project
 * Copyright (c) 2026 Fredy
 * This software is released under the MIT License.
 * See the LICENSE file in the project root for full license information.
 */
#include "Garland.h"
#include <cmath>

Garland::Garland(int pinA1, int pinA2, int channel0, int channel1, int timerNum, const char* prefsNamespace) 
    : _pinA1(pinA1), _pinA2(pinA2), _channel0(channel0), _channel1(channel1), _timerNum(timerNum),
      _mode(MODE_CONSTANT), _speed(30), 
      _manualBrightness(255), _prefsNamespace(prefsNamespace),
      _phase(0.0f), _driveMode(0), _lastUpdate(0),
      _chaosValue(0.0f), _chaosTarget(0.0f), _chaosTime(0) {
}

void Garland::begin() {
    // Завантаження налаштувань
    _prefs.begin(_prefsNamespace, false);
    _mode = _prefs.getInt("mode", MODE_CONSTANT);
    _speed = _prefs.getInt("speed", 30);
    _manualBrightness = _prefs.getInt("bright", 255);

    // Налаштовуємо таймер (1 кГц, 8 біт)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .timer_num        = (ledc_timer_t)_timerNum,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Налаштування каналів з протифазою (hpoint)
    _setupChannels();
    
    // Встановлюємо початковий стан
    if (_manualBrightness > 0) {
        _updateDuty(_manualBrightness / 255.0f, 3); // AC mode для постійного
    } else {
        _updateDuty(0.0f, 0); // OFF
    }
}

void Garland::_setupChannels() {
    uint32_t max_duty = (1 << 8) - 1; // 255 для 8-біт
    uint32_t h_point = max_duty / 2;  // 128

    // Канал 0 (Pin A1) - hpoint = 0
    ledc_channel_config_t ch0 = {
        .gpio_num       = _pinA1,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = (ledc_channel_t)_channel0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = (ledc_timer_t)_timerNum,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ch0);

    // Канал 1 (Pin A2) - hpoint = 128 (протифаза)
    ledc_channel_config_t ch1 = {
        .gpio_num       = _pinA2,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = (ledc_channel_t)_channel1,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = (ledc_timer_t)_timerNum,
        .duty           = 0,
        .hpoint         = (int)h_point
    };
    ledc_channel_config(&ch1);
}

void Garland::setMode(int mode, bool save) {
    if (_mode == mode) return;

    _mode = mode;
    if (save) {
        _prefs.putInt("mode", _mode);
    }

    // Скидання стану анімації
    _phase = 0.0f;
    _lastUpdate = millis();
    
    // Встановлюємо початковий стан для нового режиму
    if (_manualBrightness == 0) {
        _updateDuty(0.0f, 0); // Вимкнено
    } else if (_mode == MODE_CONSTANT) {
        // При перемиканні на режим Постійне встановлюємо збережену яскравість
        _updateDuty(_manualBrightness / 255.0f, 3); // AC mode
    }
}

int Garland::getMode() const {
    return _mode;
}

void Garland::setSpeed(int speed, bool save) {
    _speed = constrain(speed, 1, 100);
    if (save) {
        _prefs.putInt("speed", _speed);
    }
}

int Garland::getSpeed() const {
    return _speed;
}

void Garland::setBrightness(int brightness, bool save) {
    _manualBrightness = constrain(brightness, 0, 255);
    if (save) {
        _prefs.putInt("bright", _manualBrightness);
    }
    
    // Якщо в режимі постійного світіння, оновлюємо відразу
    if (_mode == MODE_CONSTANT) {
        if (_manualBrightness > 0) {
            _updateDuty(_manualBrightness / 255.0f, 3);
        } else {
            _updateDuty(0.0f, 0);
        }
    }
}

int Garland::getBrightness() const {
    return _manualBrightness;
}

void Garland::_updateDuty(float level, int driveMode) {
    // level: 0.0 - 1.0
    // driveMode: 0=OFF, 1=POS, 2=NEG, 3=AC
    
    uint32_t max_duty = 255; // 8-біт
    uint32_t duty = (uint32_t)(level * max_duty);
    
    if (driveMode == 0) { // OFF
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel0, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel1, 0);
    } else if (driveMode == 1) { // POS (тільки A1)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel0, duty);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel1, 0);
    } else if (driveMode == 2) { // NEG (тільки A2)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel0, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel1, duty);
    } else if (driveMode == 3) { // AC (антипаралельне, обидва з протифазою)
        uint32_t ac_duty = duty / 2;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel0, ac_duty);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel1, ac_duty);
        // hpoint вже налаштовані (0 та 128), тому вони автоматично в протифазі
    }
    
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_channel1);
    
    _driveMode = driveMode;
}

void Garland::tick() {
    // Якщо яскравість 0, нічого не робимо
    if (_manualBrightness == 0) {
        if (_driveMode != 0) {
            _updateDuty(0.0f, 0);
        }
        return;
    }
    
    // Для постійного режиму нічого не анімуємо
    if (_mode == MODE_CONSTANT) return;

    // Інтервал оновлення 20мс
    const unsigned long updateInterval = 20;
    
    if (millis() - _lastUpdate >= updateInterval) {
        _lastUpdate = millis();
        
        const float dt = 0.02f; // 20мс в секундах
        
        // Швидкість: speed 1-100 -> час циклу від 10с до 0.1с
        // speed_factor: від 0.1 до 10.0
        float speed_factor = _speed / 10.0f;
        if (speed_factor < 0.1f) speed_factor = 0.1f;
        
        // Оновлення фази
        _phase += dt / speed_factor;
        if (_phase >= 1.0f) _phase -= 1.0f;
        
        float brightness = _manualBrightness / 255.0f; // Нормалізована яскравість
        float effect_val = 0.0f;
        int drive_mode = 0;
        
        switch (_mode) {
            case MODE_CONSTANT:
                // Вже оброблено вище
                break;
                
            case MODE_ALTERNATING_SMOOTH: {
                // Почергове плавне: синусоїдальна зміна з перемиканням сторін
                effect_val = (sin(_phase * 2.0f * 2.0f * PI - 1.5707f) + 1.0f) * 0.5f;
                
                // Перемикаємо сторону кожну половину циклу
                if (_phase < 0.5f) {
                    drive_mode = 1; // POS
                } else {
                    drive_mode = 2; // NEG
                }
                break;
            }
            
            case MODE_BREATHING: {
                // Дихання: синусоїдальна зміна
                effect_val = (sin(_phase * 2.0f * PI - 1.5707f) + 1.0f) * 0.5f;
                drive_mode = 3; // AC
                break;
            }
            
            case MODE_CHAOS: {
                // Хаос: випадкові цілі з плавним переходом
                unsigned long now = millis();
                if (now - _chaosTime > (unsigned long)(speed_factor * 100)) {
                    _chaosTime = now;
                    _chaosTarget = 0.5f + (random(0, 50) / 100.0f);
                }
                
                if (_chaosValue < _chaosTarget) {
                    _chaosValue += 0.05f;
                } else {
                    _chaosValue -= 0.05f;
                }
                
                effect_val = _chaosValue;
                drive_mode = 3; // AC
                break;
            }
            
            case MODE_FLICKER: {
                // Свічка: 70% база + 30% шум
                effect_val = 0.7f + ((random(0, 100) / 100.0f - 0.5f) * 0.3f);
                effect_val = constrain(effect_val, 0.0f, 1.0f);
                drive_mode = 3; // AC
                break;
            }
        }
        
        float final_level = effect_val * brightness;
        _updateDuty(final_level, drive_mode);
    }
}
