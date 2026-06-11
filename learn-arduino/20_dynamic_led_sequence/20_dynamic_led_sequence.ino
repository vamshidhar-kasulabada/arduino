/*
 * Challenge #20: Dynamic LED Sequence
 *
 * Sketch Name:
 *
 *   20_dynamic_led_sequence
 *
 * Objective:
 *
 *   Create a reusable LED sequence engine that
 *   can work with any number of LEDs.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour:
 *
 *   One LED moves from left to right.
 *
 *       ● ○ ○
 *       ○ ● ○
 *       ○ ○ ●
 *
 *   Then starts again from the beginning.
 *
 * Requirements:
 *
 *   Create a function:
 *
 *       updateSequence(...)
 *
 *   that receives:
 *
 *       - array of LEDs
 *       - array size
 *       - current position
 *
 *   and updates the display.
 *
 * Rules:
 *
 *   - Do not hardcode LED indexes.
 *
 *   - The function should work with:
 *
 *         3 LEDs
 *         5 LEDs
 *         10 LEDs
 *
 *     without modification.
 *
 *   - Use millis().
 *
 *   - Do not use delay().
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Arrays
 *     - References
 *     - Function Parameters
 *     - Array Decay
 *
 *   Embedded:
 *     - Reusable Drivers
 *     - Non-blocking Timing
 *     - Generic Hardware Logic
 *
 * Learning Goal:
 *
 *   Learn how to write code that
 *   operates on hardware collections
 *   instead of specific hardware instances.
 */

#include "Led.h"
#include <Arduino.h>

unsigned long timer = 0;
const int interval = 300;
Led leds[] = {8, 9, 10};
int size = sizeof(leds) / sizeof(leds[0]);
int position = 0;
void setup() { timer = millis(); };

void updateSequence(Led *leds, const int size, int position) {
  for (int i = 0; i < size; i++) {
    if (i == position) {
      leds[i].on();
    } else {
      leds[i].off();
    }
  }
}

void loop() {
  if (position == size) {
    position = 0;
  }
  if (millis() - timer >= interval) {
    updateSequence(leds, size, position);
    timer += interval;
    position++;
  }
};
