/*
 * Challenge #16: Traffic Light Controller
 *
 * Sketch Name:
 *
 *   16_traffic_light_controller_class
 *
 * Objective:
 *
 *   Encapsulate the complete traffic light
 *   state machine inside a class.
 *
 * Hardware:
 *
 *   Red LED    -> Pin 8
 *   Yellow LED -> Pin 9
 *   Green LED  -> Pin 10
 *
 * Requirements:
 *
 *   Reuse your existing:
 *
 *       TrafficLight
 *
 *   class.
 *
 *   Create a new class:
 *
 *       TrafficLightController
 *
 *   that owns:
 *
 *       - TrafficLight
 *       - Current State
 *       - Timing Information
 *
 * Behaviour:
 *
 *       RED         5 seconds
 *       RED_YELLOW  2 seconds
 *       GREEN       5 seconds
 *       YELLOW      2 seconds
 *
 *       Repeat forever
 *
 * Public API:
 *
 *       update()
 *
 * Example:
 *
 *       TrafficLightController controller(
 *           8, 9, 10
 *       );
 *
 *       void loop() {
 *           controller.update();
 *       }
 *
 * Rules:
 *
 *   - Do not keep global state variables:
 *
 *         state
 *         startTime
 *
 *   - These must belong to the controller.
 *
 *   - Use millis().
 *
 *   - Do not use delay().
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Composition
 *     - Classes Owning Classes
 *     - Private State
 *     - Encapsulation
 *
 *   Embedded Systems:
 *     - Finite State Machines
 *     - Non-blocking Timing
 *     - Firmware Architecture
 *
 * Learning Goal:
 *
 *   Move from:
 *
 *       State Machine in loop()
 *
 *   to:
 *
 *       State Machine inside a class.
 *
 *   The main sketch should eventually
 *   become:
 *
 *       controller.update();
 *
 *   and nothing more.
 */

#include "TrafficLight.h"
#include <Arduino.h>

class TrafficLightController {
private:
  TrafficLight trafficLight;
  enum State { RED, RED_YELLOW, GREEN, YELLOW };
  unsigned long time;
  State state;

  void transitionTo(State nextState) {
    state = nextState;
    time = millis();
  }

public:
  TrafficLightController(uint8_t redPin, uint8_t yelloPin, uint8_t greenPin)
      : trafficLight(redPin, yelloPin, greenPin), time(millis()), state(RED) {}
  void update() {
    switch (state) {
    case RED:
      trafficLight.showRed();
      if (millis() - time >= 5000) {
        transitionTo(RED_YELLOW);
      }
      break;
    case RED_YELLOW:
      trafficLight.showYellowRed();
      if (millis() - time >= 2000) {
        transitionTo(GREEN);
      }
      break;
    case GREEN:
      trafficLight.showGreen();
      if (millis() - time >= 5000) {
        transitionTo(YELLOW);
      }
      break;
    case YELLOW:
      trafficLight.showYellow();
      if (millis() - time >= 2000) {
        transitionTo(RED);
      }
      break;
    }
  }
};

TrafficLightController controller(8, 9, 10);
void setup() {};
void loop() { controller.update(); };
