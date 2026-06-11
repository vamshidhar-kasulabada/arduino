#pragma once
#include <Arduino.h>

class Led {
private:
  uint8_t p;
  bool state;
  void write() { digitalWrite(p, state); }

public:
  Led(uint8_t pin) {
    p = pin;
    state = false;
    pinMode(p, OUTPUT);
    write();
  }

  void toggle() {
    state = !state;
    write();
  }
  void on() {
    state = true;
    write();
  }

  void off() {
    state = false;
    write();
  }

  bool isOn() { return state; }
};
