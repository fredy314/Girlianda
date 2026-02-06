#include "Garland.h"

Garland::Garland(int pinA, int pinB) 
    : _pinA(pinA), _pinB(pinB), _mode(MODE_ALTERNATING), _speed(50), 
      _manualBrightness(255), _currentBrightness(0), _fadeAmount(1), _lastUpdate(0), _maxBrightness(255) {
}

void Garland::begin() {
    // 0. Завантаження налаштувань
    _prefs.begin("garland", false);
    _mode = _prefs.getInt("mode", MODE_ALTERNATING);
    _speed = _prefs.getInt("speed", 50);
    _manualBrightness = _prefs.getInt("bright", 255);
    _maxBrightness = _manualBrightness;

    // 1. Налаштовуємо таймер (5 кГц, 8 біт)
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Початкове налаштування каналів відповідно до завантаженого режиму
    if (_mode == MODE_ALTERNATING) {
        _setupChannels(0, 128);
    } else {
        _setupChannels(0, 0);
    }
    
    // Якщо завантажили режим постійного світіння, треба відновити яскравість
    if (_mode == MODE_STEADY_ON) {
        _updateDuty(_manualBrightness);
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

    // Режим дійсно змінився
    
    // Якщо перемикаємось на/з режиму ALTERNATING, переналаштовуємо канали
    // Всі режими крім ALTERNATING повинні бути синхронні (0,0)
    bool isAlternating = (mode == MODE_ALTERNATING);
    _mode = mode;
    _prefs.putInt("mode", _mode); // Зберігаємо режим

    if (wasAlternating != isAlternating) {
        if (isAlternating) {
            _setupChannels(0, 128); // Протифаза
        } else {
            _setupChannels(0, 0);   // Синхронно
        }
        // Невелика затримка після переналаштування каналів
        delay(5);
    }

    // Скидання стану анімації
    if (_mode == MODE_OFF) {
        _updateDuty(0);
    } else if (_mode == MODE_STEADY_ON) {
        // Оновлюємо відразу, посилаючись на _maxBrightness (яке = _manualBrightness)
        _updateDuty(_maxBrightness); 
    } else {
        // Для анімацій
        _currentBrightness = 0;
        _fadeAmount = 1; 
        _lastUpdate = millis(); // Скидаємо таймер
        _updateDuty(0);
    }
}

int Garland::getMode() const {
    return _mode;
}

void Garland::setSpeed(int speed) {
    _speed = constrain(speed, 1, 100);
    _prefs.putInt("speed", _speed); // Зберігаємо швидкість
}

int Garland::getSpeed() const {
    return _speed;
}

void Garland::setBrightness(int brightness) {
    _manualBrightness = constrain(brightness, 0, 255);
    _prefs.putInt("bright", _manualBrightness); // Зберігаємо яскравість
    
    // Важливо: обмежуємо максимальну яскравість для анімацій
    _maxBrightness = _manualBrightness; 
    
    if (_mode == MODE_STEADY_ON) {
        _updateDuty(_manualBrightness);
    }
}

int Garland::getBrightness() const {
    return _manualBrightness;
}

void Garland::_updateDuty(int val) {
    // Безпечне обмеження
    val = constrain(val, 0, 255);
    
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
        delayMs = random(20, 80); // Мерехтіння має свій темп
    }

    if (millis() - _lastUpdate > delayMs) {
        _lastUpdate = millis();
        
        if (_mode == MODE_ALTERNATING || _mode == MODE_BREATHING_SYNC) {
            _currentBrightness += _fadeAmount;
            
            // Логіка відбивання від країв
            if (_currentBrightness >= _maxBrightness) {
                _currentBrightness = _maxBrightness; // Clamp
                _fadeAmount = -_fadeAmount;          // Reverse
            } else if (_currentBrightness <= 0) {
                _currentBrightness = 0;              // Clamp
                _fadeAmount = -_fadeAmount;          // Reverse
            }
            
            _updateDuty(_currentBrightness);
            
        } else if (_mode == MODE_FLICKER) {
             // Імітація свічки: База (70%) + Шум (30%)
             // Масштабуємо відносно _maxBrightness
             if (_maxBrightness > 0) {
                 int flickerBase = (_maxBrightness * 7) / 10; 
                 int flickerRange = _maxBrightness - flickerBase;
                 _currentBrightness = flickerBase + random(0, flickerRange + 1);
                 _updateDuty(_currentBrightness);
             } else {
                 _updateDuty(0);
             }
             
        } else if (_mode == MODE_CHAOS) {
            // Плавне блукання
            static int targetChaos = 0;
            
            // Якщо досягли цілі, обираємо нову в межах дозволеного
            if (abs(_currentBrightness - targetChaos) < 5) {
                targetChaos = random(0, _maxBrightness + 1);
            }
            
            // Рухаємось до цілі
            if (_currentBrightness < targetChaos) _currentBrightness += random(1, 4);
            else if (_currentBrightness > targetChaos) _currentBrightness -= random(1, 4);
            
            // Ліміти
            _currentBrightness = constrain(_currentBrightness, 0, _maxBrightness);
            
            _updateDuty(_currentBrightness);
        }
    }
}
