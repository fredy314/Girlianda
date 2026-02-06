#include "Garland.h"

Garland::Garland(int pinA, int pinB) 
    : _pinA(pinA), _pinB(pinB), _mode(MODE_ALTERNATING), _speed(50), 
      _manualBrightness(255), _currentBrightness(0), _fadeAmount(1), _lastUpdate(0), _maxBrightness(255) {
}

void Garland::begin() {
    // 1. Налаштовуємо таймер (5 кГц, 8 біт)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Початкове налаштування каналів (за замовчуванням протифаза)
    if (_mode == MODE_ALTERNATING) {
        _setupChannels(0, 128);
    } else {
        _setupChannels(0, 0);
    }
}

void Garland::_setupChannels(uint32_t hpoint0, uint32_t hpoint1) {
    // Налаштовуємо Канал 0 (Пін A)
    ledc_channel_config_t ch0 = {
        .gpio_num       = _pinA,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = (int)hpoint0
    };
    ledc_channel_config(&ch0);

    // Налаштовуємо Канал 1 (Пін B)
    ledc_channel_config_t ch1 = {
        .gpio_num       = _pinB,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = (int)hpoint1
    };
    ledc_channel_config(&ch1);
}

void Garland::setMode(int mode) {
    if (_mode == mode) return;

    // Якщо перемикаємось на/з режиму ALTERNATING, треба переналаштувати hpoint
    bool wasAlternating = (_mode == MODE_ALTERNATING);
    bool isAlternating = (mode == MODE_ALTERNATING);

    _mode = mode;

    if (wasAlternating != isAlternating) {
        if (isAlternating) {
            _setupChannels(0, 128); // Протифаза
        } else {
            _setupChannels(0, 0);   // Синхронно
        }
    }

    // Скидання стану анімації
    if (_mode == MODE_OFF) {
        _updateDuty(0);
    } else if (_mode == MODE_STEADY_ON) {
        _updateDuty(_manualBrightness);
    } else {
        // Для анімацій починаємо з 0
        _currentBrightness = 0;
        _fadeAmount = 1; 
        _updateDuty(0);
    }
}

int Garland::getMode() const {
    return _mode;
}

void Garland::setSpeed(int speed) {
    _speed = constrain(speed, 1, 100);
}

int Garland::getSpeed() const {
    return _speed;
}

void Garland::setBrightness(int brightness) {
    _manualBrightness = constrain(brightness, 0, 255);
    if (_mode == MODE_STEADY_ON) {
        _updateDuty(_manualBrightness);
    }
}

int Garland::getBrightness() const {
    return _manualBrightness;
}

void Garland::_updateDuty(int val) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

void Garland::tick() {
    if (_mode == MODE_OFF || _mode == MODE_STEADY_ON) return;

    // Розрахунок затримки
    int delayMs = map(101 - _speed, 1, 100, 2, 40); 
    
    // Специфічні налаштування для нових режимів
    if (_mode == MODE_FLICKER) {
        delayMs = random(10, 50); // Випадкова швидкість для мерехтіння
    } else if (_mode == MODE_CHAOS) {
        // У Хаосі швидкість може змінюватись динамічно, але поки використовуємо базову
    }

    if (millis() - _lastUpdate > delayMs) {
        _lastUpdate = millis();
        
        if (_mode == MODE_ALTERNATING || _mode == MODE_BREATHING_SYNC) {
            _currentBrightness += _fadeAmount;
            if (_currentBrightness <= 0 || _currentBrightness >= _maxBrightness) {
                _fadeAmount = -_fadeAmount;
                if (_currentBrightness < 0) _currentBrightness = 0;
                if (_currentBrightness > _maxBrightness) _currentBrightness = _maxBrightness;
            }
            _updateDuty(_currentBrightness);
            
        } else if (_mode == MODE_FLICKER) {
             // Імітація свічки: база + випадкове відхилення
             int flickerBase = 150; 
             int flickerVar = random(0, 105);
             _currentBrightness = flickerBase + flickerVar;
             _updateDuty(_currentBrightness);
             
        } else if (_mode == MODE_CHAOS) {
            // Плавний перехід до випадкової цілі
            static int targetChaos = 128;
            
            if (_currentBrightness < targetChaos) _currentBrightness += random(1, 5);
            else if (_currentBrightness > targetChaos) _currentBrightness -= random(1, 5);
            
            // Якщо досягли цілі (або близько), обираємо нову
            if (abs(_currentBrightness - targetChaos) < 5) {
                targetChaos = random(10, _maxBrightness);
            }
            _updateDuty(_currentBrightness);
        }
    }
}
