/*
 * Challenge #9: LED Class
 *
 * Objective:
 * Create a reusable LED class that encapsulates
 * the behavior of a single LED.
 *
 * Hardware:
 *
 *   Red LED    -> Pin 8
 *   Yellow LED -> Pin 9
 *   Green LED  -> Pin 10
 *
 * Requirements:
 *
 *   Create a class named Led.
 *
 *   The class should:
 *
 *     - Store the LED pin number.
 *     - Configure the pin as OUTPUT.
 *     - Turn the LED ON.
 *     - Turn the LED OFF.
 *     - Toggle the LED state.
 *
 * Example Usage:
 *
 *   Led red(8);
 *   Led yellow(9);
 *   Led green(10);
 *
 *   red.on();
 *   yellow.off();
 *   green.toggle();
 *
 * Behavior:
 *
 *   Every second:
 *
 *     1. Toggle Red LED
 *     2. Toggle Yellow LED
 *     3. Toggle Green LED
 *
 * Rules:
 *
 *   - Do not call digitalWrite()
 *     outside the Led class.
 *
 *   - All LED manipulation should happen
 *     through class methods.
 *
 * Concepts Practiced:
 *
 *   - Classes
 *   - Constructors
 *   - Objects
 *   - Encapsulation
 *   - Hardware Abstraction
 *   - Non-blocking Timing (millis)
 */

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

Led redLed(8); //same as Led redLed = Led(8);
Led yellowLed = Led(9);
Led greenLed = Led(10);
unsigned long time = 0;
void setup() { time = millis(); }
void loop() {
  if (millis() - time >= 1000) {
    time = millis();
    redLed.toggle();
    yellowLed.toggle();
    greenLed.toggle();
  }
}
