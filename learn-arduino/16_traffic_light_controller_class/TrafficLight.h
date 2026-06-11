#pragma once
#include "Led.h"
#include <Arduino.h>
class TrafficLight {

private:
  Led redLed;
  Led yellowLed;
  Led greenLed;

public:
  TrafficLight(uint8_t redPin, uint8_t yelloPin, uint8_t greenPin)
      : redLed(redPin), yellowLed(yelloPin), greenLed(greenPin) {}

  void showRed() {
    redLed.on();
    yellowLed.off();
    greenLed.off();
  }

  void showYellow() {
    redLed.off();
    yellowLed.on();
    greenLed.off();
  }

  void showGreen() {
    redLed.off();
    yellowLed.off();
    greenLed.on();
  }

  void showYellowRed() {
    redLed.on();
    yellowLed.on();
    greenLed.off();
  }
};
