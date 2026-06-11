/*
 * Challenge #8: Knight Rider LED Scanner
 *
 * Objective:
 * Create a moving LED scanner effect similar to the
 * Knight Rider car or Cylon eye effect.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *   LED4 -> Pin 11
 *   LED5 -> Pin 12
 *
 * Behaviour:
 *
 *   ● ○ ○ ○ ○
 *   ○ ● ○ ○ ○
 *   ○ ○ ● ○ ○
 *   ○ ○ ○ ● ○
 *   ○ ○ ○ ○ ●
 *   ○ ○ ○ ● ○
 *   ○ ○ ● ○ ○
 *   ○ ● ○ ○ ○
 *   ● ○ ○ ○ ○
 *   (repeat forever)
 *
 * Requirements:
 *
 *   - Use millis() for timing.
 *   - Do not use delay().
 *   - Use an array to store LED pins.
 *   - Use a loop when updating LEDs.
 *   - Only one LED should be ON at a time.
 *
 * Suggested Variables:
 *
 *   currentLed
 *   direction
 *   previousUpdateTime
 *
 * Concepts Practiced:
 *
 *   - Arrays
 *   - Loops
 *   - Indexing
 *   - Boundary Conditions
 *   - State Representation
 *   - Non-blocking Programming
 */


#include <Arduino.h>

enum Direction {
  RIGHT,
  LEFT,
};

uint8_t leds[] = {8, 9, 10};
uint8_t size = sizeof(leds) / sizeof(leds[0]);
int led = 0;
unsigned long time = 0;
Direction direction = RIGHT;

void onLed(const uint8_t leds[], uint8_t ledToOn, uint8_t size) {
  for (int i = 0; i < size; i++) {
    if (i == ledToOn) {
      digitalWrite(leds[i], HIGH);
    } else {
      digitalWrite(leds[i], LOW);
    }
  }
}

void setup() {
  for (uint8_t pin : leds) {
    pinMode(pin, OUTPUT);
  }
  time = millis();
}

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
