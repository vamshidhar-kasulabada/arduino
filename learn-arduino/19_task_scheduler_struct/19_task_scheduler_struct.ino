/*
 * Challenge #19: Task Scheduler Using Structs
 *
 * Sketch Name:
 *
 *   19_task_scheduler_struct
 *
 * Objective:
 *
 *   Refactor the multi-timer scheduler to
 *   use a struct instead of parallel arrays.
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
 *   LED2 toggles every 1000ms
 *   LED3 toggles every 2000ms
 *
 * Requirements:
 *
 *   Create a struct that stores:
 *
 *       - Led
 *       - Timer
 *       - Interval
 *
 * Example:
 *
 *   struct Task {
 *       ...
 *   };
 *
 * Create an array of tasks.
 *
 * The scheduler should iterate through
 * the array and update each task.
 *
 * Rules:
 *
 *   - Use millis().
 *   - Do not use delay().
 *   - Do not use separate arrays:
 *
 *         leds[]
 *         timers[]
 *         intervals[]
 *
 *   - Use a single array of structs.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Structs
 *     - Arrays of Structs
 *     - Data Modeling
 *     - Aggregation
 *
 *   Embedded:
 *     - Task Scheduling
 *     - Scalable Firmware Design
 *     - Periodic Tasks
 *
 * Learning Goal:
 *
 *   Learn how to group related data
 *   together.
 *
 * Move from:
 *
 *   leds[i]
 *   timers[i]
 *   intervals[i]
 *
 * to:
 *
 *   tasks[i]
 *
 * where all information required to
 * execute a task lives in one place.
 */


#include "Led.h"
#include <Arduino.h>

struct Task {
  Led led;
  unsigned long timer;
  unsigned long interval;
};

Task tasks[] = {{8, 0, 500}, {9, 0, 1000}, {10, 0, 2000}};
void setup() {

};
void loop() {
  for (Task &task : tasks) {
    if (millis() - task.timer >= task.interval) {
      task.led.toggle();
      task.timer += task.interval;
    }
  }
};
