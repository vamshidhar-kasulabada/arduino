#pragma once
#include <Arduino.h>

class Led {
private:
  uint8_t pin;
  bool state;
  void write() { digitalWrite(pin, state); }

public:
  Led(uint8_t p) {
    pin = p;
    state = false;
    pinMode(pin, OUTPUT);
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
