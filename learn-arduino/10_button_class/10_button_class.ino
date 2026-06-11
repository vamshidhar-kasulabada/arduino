/*
 * Challenge #10: Button Class with Debouncing
 *
 * Objective:
 * Create a reusable Button class that encapsulates
 * button handling and debounce logic.
 *
 * Hardware:
 *
 *   Push Button -> Pin 2
 *   LED         -> Pin 8
 *
 * Requirements:
 *
 *   Create a Button class that:
 *
 *     - Stores the button pin.
 *     - Configures the pin as INPUT_PULLUP.
 *     - Detects button press events.
 *     - Handles switch bouncing internally.
 *     - Exposes a clean API to the user.
 *
 * Example Usage:
 *
 *   Button button(2);
 *
 *   if (button.wasPressed()) {
 *       ...
 *   }
 *
 * Debounce Requirements:
 *
 *   - Do not use delay().
 *   - Use millis().
 *   - Implement stable-state debounce.
 *   - A press should only be reported after
 *     the signal remains stable for at least
 *     20ms.
 *
 * Example:
 *
 *   Raw Signal:
 *
 *     HIGH
 *     LOW
 *     HIGH
 *     LOW
 *     HIGH
 *     LOW
 *     LOW
 *     LOW
 *
 *   Debounced Signal:
 *
 *     HIGH
 *     LOW
 *
 *   Only one press event should be generated.
 *
 * Test Program:
 *
 *   - Create a Led object.
 *   - Create a Button object.
 *   - Toggle the LED whenever the button
 *     is pressed.
 *
 * Rules:
 *
 *   - Application code should never directly
 *     use digitalRead().
 *   - Application code should never directly
 *     implement debounce logic.
 *   - All button handling should happen
 *     inside the Button class.
 *
 * Concepts Practiced:
 *
 *   - Classes
 *   - Constructors
 *   - Encapsulation
 *   - State Management
 *   - Debouncing
 *   - Hardware Abstraction
 *   - Event Detection
 *   - Non-blocking Programming
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

Button button(2);
Led redLed(8);

void setup() { Serial.begin(9600); }

void loop() {
  if (button.wasPressed()) {
    redLed.toggle();
  }
}
