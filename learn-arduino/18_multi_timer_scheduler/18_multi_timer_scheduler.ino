/*
 * Challenge #18: Multi Timer Scheduler
 *
 * Sketch Name:
 *
 *   18_multi_timer_scheduler
 *
 * Objective:
 *
 *   Run multiple independent tasks at
 *   different intervals without using delay().
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour:
 *
 *   LED1 toggles every 500ms
 *
 *   LED2 toggles every 1000ms
 *
 *   LED3 toggles every 2000ms
 *
 * All LEDs must operate independently.
 *
 * Example:
 *
 *   Time    LED1 LED2 LED3
 *   ----------------------
 *   0ms      OFF  OFF  OFF
 *   500ms    ON   OFF  OFF
 *   1000ms   OFF  ON   OFF
 *   1500ms   ON   ON   OFF
 *   2000ms   OFF  OFF  ON
 *
 * Requirements:
 *
 *   - Use your Led class.
 *   - Use millis().
 *   - Do not use delay().
 *
 * Rules:
 *
 *   - Each LED must have its own timer.
 *   - The timing of one LED must not
 *     affect the others.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Arrays
 *     - Classes
 *     - Structuring Repeated Logic
 *
 *   Embedded:
 *     - Cooperative Scheduling
 *     - Multiple Timers
 *     - Concurrent Tasks
 *
 * Learning Goal:
 *
 *   Understand how embedded systems
 *   perform many periodic tasks at the
 *   same time without an operating system.
 */

#include "Led.h"
#include <Arduino.h>

/*
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;

Led led1(8);
Led led2(9);
Led led3(10);
void setup() {};
void loop() {
  if (millis() - timer1 >= 500) {
    led1.toggle();
    timer1 +=500;
  }
  if (millis() - timer2 >= 1000) {
    led2.toggle();
    timer2 +=1000();
  }
  if (millis() - timer3 >= 2000) {
    led3.toggle();
    timer3 +=2000;
  }
};
*/

Led leds[] = {8, 9, 10};
const uint8_t size = sizeof(leds) / sizeof(leds[0]);
unsigned long timers[] = {0, 0, 0};
unsigned long intervals[] = {500, 1000, 2000};
void setup() {};
void loop() {
  for (int i = 0; i < size; i++) {
    if (millis() - timers[i] >= intervals[i]) {
      leds[i].toggle();
      timers[i] += intervals[i];
    }
  }
};
