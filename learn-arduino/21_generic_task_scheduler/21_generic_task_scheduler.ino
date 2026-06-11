/*
 * Challenge #21: Generic Task Scheduler
 *
 * Sketch Name:
 *
 *   21_generic_task_scheduler
 *
 * Objective:
 *
 *   Build a reusable scheduler that can
 *   manage multiple LED tasks.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour:
 *
 *   Each LED toggles according to its
 *   configured interval.
 *
 * Example:
 *
 *   LED1 -> 500ms
 *   LED2 -> 1000ms
 *   LED3 -> 2000ms
 *
 * Requirements:
 *
 *   Create:
 *
 *       struct Task
 *
 *   containing:
 *
 *       Led led;
 *       unsigned long timer;
 *       unsigned long interval;
 *
 *   Then create:
 *
 *       class Scheduler
 *
 *   that owns:
 *
 *       Task* tasks;
 *       uint8_t taskCount;
 *
 * Public API:
 *
 *       update();
 *
 * Example:
 *
 *       Scheduler scheduler(tasks, size);
 *
 *       void loop() {
 *           scheduler.update();
 *       }
 *
 * Rules:
 *
 *   - Scheduler must not know how many
 *     tasks exist at compile time.
 *
 *   - Pass the task array into the
 *     constructor.
 *
 *   - Use pointers to access tasks.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Structs
 *     - Pointers
 *     - Classes
 *     - Constructors
 *     - Composition
 *
 *   Embedded:
 *
 *     - Cooperative Scheduling
 *     - Firmware Architecture
 *     - Generic Components
 *
 * Learning Goal:
 *
 *   Build a reusable firmware component
 *   that operates on user-provided data.
 *
 *   This is similar to how many embedded
 *   frameworks are designed.
 */


#include <Arduino.h>
#include "Led.h"

void setup() {};
void loop() {};
