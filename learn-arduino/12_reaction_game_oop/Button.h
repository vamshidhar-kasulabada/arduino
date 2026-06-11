#pragma once
#include <Arduino.h>
class Button {
private:
  uint8_t pin;
  bool stableState;
  bool lastReading;
  unsigned long lastChangeTime;

public:
  Button(uint8_t p)
      : pin(p), stableState(HIGH), lastReading(HIGH), lastChangeTime(0) {
    pinMode(pin, INPUT_PULLUP);
  }

  bool wasPressed() {
    bool currentReading = digitalRead(pin);

    if (currentReading != lastReading) {
      lastChangeTime = millis();
      lastReading = currentReading;
    }

    // Has it remained unchanged for 20ms?
    if (millis() - lastChangeTime >= 20) {

      // Has the stable state changed?
      if (stableState != currentReading) {
        stableState = currentReading;

        // HIGH -> LOW means button press
        if (stableState == LOW) {
          return true;
        }
      }
    }

    return false;
  }
};
