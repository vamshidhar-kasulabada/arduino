/*
 * Challenge #23: Polymorphic Task Scheduler
 *
 * Sketch Name:
 *
 *   23_polymorphic_scheduler
 *
 * Objective:
 *
 *   Rebuild the scheduler using inheritance and
 *   virtual functions instead of function pointers.
 *
 *   Each task becomes an OBJECT that carries its own
 *   data and overrides a virtual run() method. The
 *   scheduler calls run() without knowing which kind
 *   of task it is.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Behaviour (same as Challenge 22 -- the point is the
 * ARCHITECTURE, not new behaviour):
 *
 *   Blink LED1            every 500ms
 *   Blink LED2            every 1000ms
 *   Print "tick" to Serial every 2000ms
 *
 * Requirements:
 *
 *   1. An ABSTRACT base class:
 *
 *          class Task {
 *          protected:
 *              unsigned long timer;
 *              unsigned long interval;
 *          public:
 *              Task(unsigned long interval);
 *              virtual void run() = 0;   // pure virtual: WHAT
 *              void update();            // shared timing: WHEN
 *          };
 *
 *      update() checks the interval and calls run().
 *      run() is pure virtual -- the base does NOT
 *      define it; every subclass must.
 *
 *   2. CONCRETE subclasses, each owning its own data:
 *
 *          class BlinkTask : public Task {
 *              Led led;                 // carries its own LED
 *              ...
 *              void run() override;     // led.toggle()
 *          };
 *
 *          class PrintTask : public Task {
 *              const char* msg;         // carries its own message
 *              ...
 *              void run() override;     // Serial.println(msg)
 *          };
 *
 *   3. A Scheduler that owns an array of BASE pointers
 *      (Task*) and calls update() on each:
 *
 *          Task* tasks[] = { &blink1, &blink2, &printer };
 *
 * Public API:
 *
 *          scheduler.update();
 *
 * Rules:
 *
 *   - The Scheduler must work with ANY Task subclass,
 *     accessed only through Task* base pointers. It
 *     must contain no LED code and no Serial code.
 *
 *   - No function pointers this time -- use virtual
 *     dispatch.
 *
 *   - Use millis(). Do not use delay().
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Inheritance ( : public Task )
 *     - Virtual Functions
 *     - Pure Virtual / Abstract Base Class ( = 0 )
 *     - Polymorphism (base pointer -> derived behaviour)
 *     - override keyword
 *     - Virtual Destructor
 *     - Template Method pattern (update() calls run())
 *
 *   Embedded:
 *     - Polymorphic firmware components
 *     - Plugin-style architecture
 *     - The OOP form of callbacks
 *
 * Learning Goal:
 *
 *   This is the OOP answer to Challenge 22's wall.
 *
 *   In 22 a function pointer could not carry data, so
 *   every callback had to reach a GLOBAL led. Here each
 *   task is an object that OWNS its data (its Led, its
 *   message) while still presenting one uniform
 *   interface -- run(). The scheduler calls run() on a
 *   Task* and the correct subclass's version executes,
 *   chosen at runtime by the vtable.
 *
 *   Compare the two sketches side by side: function
 *   pointers vs virtual functions are two routes to the
 *   same goal -- separating WHEN from WHAT.
 *
 * Stretch Goals (optional):
 *
 *   - Add a CountdownTask that runs N times then stops.
 *   - Add enable() / disable() to the base Task.
 *   - On AVR, each object with a virtual function carries
 *     a hidden vtable pointer (2 bytes RAM). Note the
 *     RAM difference vs the function-pointer version.
 */

#include "Led.h"
#include <Arduino.h>

// ===========================================================================
// Abstract base class: the INTERFACE + shared timing every task has.
// You cannot create a `Task` directly -- it has a pure virtual method.
// ===========================================================================
class Task {
protected:
  unsigned long timer;
  unsigned long interval;

public:
  Task(unsigned long intervalMs) : timer(0), interval(intervalMs) {}

  // Pure virtual ( = 0 ): no body. Each subclass MUST define run().
  // This is "WHAT the task does" -- deliberately left open.
  virtual void run() = 0;

  // Non-virtual shared timing ("WHEN"). It calls run(), and at runtime the
  // CORRECT subclass's run() executes. That is polymorphism in action.
  void update() {
    if (millis() - timer >= interval) {
      run();
      timer += interval;
    }
  }

  // Virtual destructor: lets `delete somethingViaTaskPointer` clean up the
  // real subclass correctly. Good habit whenever a class has virtual methods.
  virtual ~Task() {}
};

// ===========================================================================
// TODO 1: Concrete tasks. Each OWNS its own data and overrides run().
//
//   class BlinkTask : public Task {
//     Led led;
//   public:
//     BlinkTask(uint8_t pin, unsigned long interval) : Task(interval), led(pin)
//     {} void run() override { /* toggle the led */ }
//   };
//
//   class PrintTask : public Task { const char* msg; ... };
// ===========================================================================

// ===========================================================================
// TODO 2: A Scheduler that holds Task* (base pointers) and calls update()
//         on each. It must work for ANY Task subclass.
// ===========================================================================

// ===========================================================================
// TODO 3: Create your task objects, collect their addresses in a Task* array,
//         build the Scheduler, and drive it from loop().
// ===========================================================================
class BlinkTask : public Task {
  Led led;

public:
  BlinkTask(uint8_t pin, unsigned long interval) : Task(interval), led(pin) {}
  void run() override { led.toggle(); }
};

class PrintTask : public Task {
  const char *_msg;

public:
  PrintTask(const char *msg, unsigned long interval)
      : Task(interval), _msg(msg) {}
  void run() override { Serial.println(_msg); }
};

class Scheduler {
  Task **_tasks;
  int _count;

public:
  Scheduler(Task **tasks, int count) : _tasks(tasks), _count(count) {};
  void update() {
    for (int i = 0; i < _count; i++) {
      _tasks[i]->update();
    }
  }
};
BlinkTask blink1(8, 500);
BlinkTask blink2(9, 1000);
PrintTask printer("tick", 2000);

Task *tasks[] = {&blink1, &blink2, &printer};
const int count = sizeof(tasks) / sizeof(tasks[0]);
void setup() { Serial.begin(9600); }
Scheduler scheduler(tasks, 3);

void loop() { scheduler.update(); }
