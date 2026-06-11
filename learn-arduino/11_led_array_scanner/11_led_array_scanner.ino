/*
 * Challenge #11: LED Array Scanner (OOP Version)
 *
 * Sketch Name:
 *
 *   11_led_array_scanner
 *
 * Objective:
 *
 *   Recreate the Knight Rider / Cylon scanner effect
 *   using an array of Led objects instead of directly
 *   controlling Arduino pins.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour:
 *
 *   ● ○ ○
 *   ○ ● ○
 *   ○ ○ ●
 *   ○ ● ○
 *   ● ○ ○
 *   (repeat forever)
 *
 * Requirements:
 *
 *   - Create a reusable Led class.
 *   - Create an array of Led objects.
 *   - Only one LED should be ON at a time.
 *   - LEDs should move left to right and then
 *     right to left continuously.
 *   - Use millis() for timing.
 *   - Do not use delay().
 *
 * Example:
 *
 *   Led leds[] = {
 *       Led(8),
 *       Led(9),
 *       Led(10)
 *   };
 *
 * State Variables:
 *
 *   currentLed
 *   direction
 *   previousUpdateTime
 *
 * Rules:
 *
 *   - Do not call digitalWrite() outside
 *     the Led class.
 *
 *   - Do not call pinMode() outside
 *     the Led class.
 *
 *   - All LED operations must happen through:
 *
 *       led.on();
 *       led.off();
 *       led.toggle();
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Classes
 *     - Constructors
 *     - Arrays of Objects
 *     - Functions
 *     - Encapsulation
 *
 *   Embedded Systems:
 *     - Hardware Abstraction
 *     - Non-blocking Timing
 *     - State Management
 *     - Reusable Components
 *
 * Learning Goal:
 *
 *   Move from thinking in terms of:
 *
 *       Pin 8
 *       Pin 9
 *       Pin 10
 *
 *   to thinking in terms of:
 *
 *       leds[0]
 *       leds[1]
 *       leds[2]
 *
 *   and from:
 *
 *       digitalWrite(...)
 *
 *   to:
 *
 *       led.on()
 *       led.off()
 *
 *   which is a foundational step toward
 *   object-oriented embedded programming.
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

enum Direction {
  RIGHT,
  LEFT,
};
Led leds[] = {Led(8), Led(9), Led(10)};
uint8_t size = sizeof(leds) / sizeof(leds[0]);
int led = 0;
unsigned long time = 0;
Direction direction = RIGHT;

void onLed(Led leds[], uint8_t ledToOn, uint8_t size) {
  for (int i = 0; i < size; i++) {
    if (i == ledToOn) {
      leds[i].on();
    } else {
      leds[i].off();
    }
  }
}
void setup() {}
void loop() {

  if (millis() - time >= 300) {
    time = millis();
    onLed(leds, led, size);
    switch (direction) {
    case RIGHT:
      led++;
      if (led == size) {
        direction = LEFT;
        led = size - 2;
      }
      break;
    case LEFT:
      led--;
      if (led == -1) {
        direction = RIGHT;
        led = 1;
      }
      break;
    }
  }
}
