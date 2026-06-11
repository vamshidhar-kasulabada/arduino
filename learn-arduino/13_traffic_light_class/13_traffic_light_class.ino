/*
 * Challenge #13: Traffic Light Class
 *
 * Sketch Name:
 *
 *   13_traffic_light_class
 *
 * Objective:
 *
 *   Create a TrafficLight class that encapsulates
 *   three LEDs and exposes methods for controlling
 *   a complete traffic light.
 *
 * Hardware:
 *
 *   Red LED    -> Pin 8
 *   Yellow LED -> Pin 9
 *   Green LED  -> Pin 10
 *
 * Requirements:
 *
 *   Create a TrafficLight class.
 *
 *   The class must internally own:
 *
 *       Led red;
 *       Led yellow;
 *       Led green;
 *
 *   The class should expose methods such as:
 *
 *       showRed()
 *       showYellow()
 *       showGreen()
 *       showRedYellow()
 *
 * Behaviour:
 *
 *   Implement the following cycle:
 *
 *       RED         5 seconds
 *       RED_YELLOW  2 seconds
 *       GREEN       5 seconds
 *       YELLOW      2 seconds
 *
 *       Repeat forever
 *
 * Rules:
 *
 *   - Use millis().
 *   - Do not use delay().
 *   - Do not call digitalWrite() directly.
 *   - Use the Led class exclusively.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Composition
 *     - Constructors
 *     - Member Initialization
 *     - Encapsulation
 *
 *   Embedded:
 *     - State Machines
 *     - Hardware Abstraction
 *     - Reusable Components
 *
 * Learning Goal:
 *
 *   Move from:
 *
 *       Led
 *
 *   to:
 *
 *       TrafficLight
 *
 *   where a higher-level object is built
 *   from lower-level objects.
 */

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

enum State {
  RED,
  YELLOW,
  GREEN,
  YELLOW_RED,
};

State state = RED;
unsigned long time = 0;
TrafficLight trafficLight(8, 9, 10);

void setup() { time = millis(); };

void loop() {
  switch (state) {
  case RED:
    trafficLight.showRed();
    if (millis() - time >= 5000) {
      state = YELLOW_RED;
      time = millis();
    }
    break;
  case YELLOW_RED:
    trafficLight.showYellowRed();
    if (millis() - time >= 2000) {
      state = GREEN;
      time = millis();
    }
    break;
  case GREEN:
    trafficLight.showGreen();
    if (millis() - time >= 5000) {
      state = YELLOW;
      time = millis();
    }
    break;
  case YELLOW:
    trafficLight.showYellow();
    if (millis() - time >= 2000) {
      state = RED;
      time = millis();
    }
    break;
  }
};
