#ifndef REMOTEXY_INTERFACE_H
#define REMOTEXY_INTERFACE_H

#include <Arduino.h>
#include "Garland.h"

class RemoteXYInterface {
public:
    RemoteXYInterface(Garland& garlandA, Garland& garlandB);
    
    void begin();
    void tick();
    
    bool isConnected() const;

private:
    Garland& _garlandA;
    Garland& _garlandB;
    
    // Попередні значення для відстеження змін
    uint8_t _lastGarlandSwitch;
    uint8_t _lastModeSwitch;
    int8_t _lastSpeedSlider;
    int8_t _lastBrightnessSlider;
    
    // Вказівник на активну гірлянду
    Garland* _activeGarland;
    
    void _updateActiveGarland();
    void _syncFromGarland();
    void _applyChanges();
};

#endif
