/*
 * Challenge #22: Callback Task Scheduler
 *
 * Sketch Name:
 *
 *   22_callback_scheduler
 *
 * Objective:
 *
 *   Upgrade the scheduler so each task runs a
 *   user-defined function (a "callback") at its
 *   interval -- instead of hardcoding LED toggling.
 *
 *   The scheduler should no longer know, or care,
 *   what a task actually does.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour:
 *
 *   Task A -> toggle LED1             every 500ms
 *   Task B -> toggle LED2             every 1000ms
 *   Task C -> print "tick" to Serial  every 2000ms
 *
 *   Task C is the important one: it proves the
 *   scheduler is generic. A task does NOT have to
 *   be an LED -- it can be anything.
 *
 * Requirements:
 *
 *   1. Define a callback type -- a pointer to a
 *      function that takes no arguments and
 *      returns nothing:
 *
 *          typedef void (*TaskCallback)();
 *
 *   2. Create a struct that stores the callback
 *      instead of an Led:
 *
 *          struct Task {
 *              TaskCallback action;
 *              unsigned long timer;
 *              unsigned long interval;
 *          };
 *
 *   3. Create a Scheduler class that owns the
 *      task array and runs each task when due:
 *
 *          class Scheduler {
 *              Task*   tasks;
 *              uint8_t count;
 *          public:
 *              void update();   // call action() on every due task
 *          };
 *
 *   4. Write ordinary functions and hand them in
 *      as callbacks:
 *
 *          void blinkLed1() { led1.toggle(); }
 *
 *          Task tasks[] = {
 *              { blinkLed1, 0, 500  },
 *              { blinkLed2, 0, 1000 },
 *              { printTick, 0, 2000 },
 *          };
 *
 * Public API:
 *
 *          scheduler.update();
 *
 * Rules:
 *
 *   - The Scheduler class must contain NO LED code
 *     and NO Serial code. It may only call
 *     task.action(). All "what to do" lives in the
 *     callbacks, not the scheduler.
 *
 *   - Use millis(). Do not use delay().
 *
 *   - The same Scheduler must work unchanged whether
 *     a callback blinks an LED or prints to Serial.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Function Pointers
 *     - Callbacks
 *     - typedef / type aliases
 *     - Separation of Concerns
 *
 *   Embedded:
 *     - Event-driven Scheduling
 *     - Decoupled Firmware Architecture
 *     - Cooperative Multitasking
 *     - Library-style Component Design
 *
 * Learning Goal:
 *
 *   Separate WHEN a task runs (the scheduler) from
 *   WHAT a task does (the callback).
 *
 *   In lesson 21 the scheduler was glued to the Led
 *   class. Here it becomes a pure timing engine that
 *   could drive motors, sensors, or radios without a
 *   single line changing. This is exactly how timer
 *   libraries, RTOS tasks, and interrupt service
 *   routines are wired.
 *
 * Stretch Goals (optional):
 *
 *   - Add enable() / disable() per task.
 *   - Add a "one-shot" task: it runs once, then
 *     disables itself.
 *   - Think: how would you pass DATA to a callback
 *     (e.g. which pin to toggle)? A plain function
 *     pointer can't capture state -- that limitation
 *     is what leads to std::function and lambdas in a
 *     later lesson.
 */

#include "Led.h"
#include <Arduino.h>

// --- Hardware ------------------------------------------------------------
Led led1(8);
Led led2(9);
Led led3(10);

// --- Callback type -------------------------------------------------------
// Read this right-to-left: TaskCallback is a pointer (*) to a function
// taking () no args and returning void.
typedef void (*TaskCallback)();

// --- Your callbacks: the "WHAT" ------------------------------------------
// TODO: implement these three free functions.
void blinkLed1() { led1.toggle(); }
void blinkLed2() { led2.toggle(); }
void printTick() { Serial.println("tick"); }

// -- TODO ----------------------------------------------------------------
// 1. struct Task { TaskCallback action; ... };
// 2. class Scheduler { ... void update(); };
// 3. Build tasks[] (pass the function NAMES, no parentheses) and a Scheduler.

struct Task {
  TaskCallback action;
  unsigned long timer;
  unsigned long interval;
};

class Scheduler {
private:
  Task *t;
  int c;

public:
  Scheduler(Task *tasks, int count) : t(tasks), c(count) {}
  void update() {
    for (int i = 0; i < c; i++) {
      Task *task = t + i;
      if (millis() - task->timer >= task->interval) {
        task->action();
        task->timer += task->interval;
      }
    }
  }
  // void update() {
  //   for (int i = 0; i < c; i++) {
  //     if (millis() - t[i].timer >= t[i].interval) {
  //       t[i].action();
  //       t[i].timer += t[i].interval;
  //     }
  //   }
  // }
};

Task tasks[] = {
    {blinkLed1, 0, 500}, {blinkLed2, 0, 1000}, {printTick, 0, 2000}};
const uint8_t count = sizeof(tasks) / sizeof(tasks[0]);
Scheduler scheduler(tasks, count);

void setup() { Serial.begin(9600); }
void loop() { scheduler.update(); }
