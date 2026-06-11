# Function Pointers & Callbacks (Challenge #22)

A companion to `22_callback_scheduler.ino`. The scheduler in this challenge works
because of **function pointers** — this note explains them from the ground up, then
ties each idea back to the code you wrote.

---

## 1. The core idea

A normal pointer stores the *address of a variable*. A **function pointer** stores the
*address of a function* — so you can pass a function around like data, store it, and
call it later.

```cpp
void greet() { Serial.println("hi"); }

void (*fp)() = greet;   // fp now holds the address of greet
fp();                   // calls greet()  -> prints "hi"
```

Two things to notice:

- `greet` (no parentheses) means **"the function itself"** (its address).
- `greet()` (with parentheses) means **"call it now"**.

That single distinction is the whole game. You pass `greet`; the other code decides
when to do `greet()`.

---

## 2. Reading the declaration

```cpp
void (*fp)();
//   ^   ^  ^
//   |   |  └── () : takes no arguments
//   |   └────── fp : the name of the pointer
//   └────────── void : the function it points to returns void
```

The parentheses around `*fp` are **required**. Without them the meaning flips:

```cpp
void (*fp)();   // fp is a POINTER to a function returning void
void  *fp();    // fp is a FUNCTION returning a void*   <-- completely different!
```

With parameters, the types go in the inner `()`:

```cpp
int (*op)(int, int);    // pointer to a function: int f(int, int)

int add(int a, int b) { return a + b; }
op = add;
int x = op(2, 3);       // 5
```

---

## 3. Giving the type a name (`typedef` / `using`)

Raw function-pointer syntax is ugly, so we name the type. This is the line from your
sketch:

```cpp
typedef void (*TaskCallback)();   // TaskCallback == "pointer to void f()"
```

Now `TaskCallback` is a type you can use like any other:

```cpp
TaskCallback cb = greet;   // much easier to read
cb();
```

The modern C++ equivalent (same meaning, often preferred):

```cpp
using TaskCallback = void (*)();
```

---

## 4. Callbacks: handing a function to other code

A **callback** is just a function pointer you give to someone else so *they* can
"call you back" later. That's what makes the scheduler generic:

```cpp
void runTwice(TaskCallback cb) {
  cb();
  cb();
}

runTwice(greet);   // pass the function; runTwice decides when to call it
```

`runTwice` has no idea what `cb` does — print, blink, fire a rocket. It only knows the
*shape* (`void()`). This is **inversion of control**: the caller supplies the behavior.

---

## 5. How this powers your scheduler

In lesson 21 the scheduler was hardwired to `led.toggle()`. Here, each task stores a
**callback** instead:

```cpp
struct Task {
  TaskCallback action;     // WHAT to do  (a function pointer)
  unsigned long timer;     // WHEN it last ran
  unsigned long interval;  // how often
};
```

You build the table by passing function **names** (no parentheses — you're storing the
functions, not calling them):

```cpp
void blinkLed1() { led1.toggle(); }
void printTick() { Serial.println("tick"); }

Task tasks[] = {
  { blinkLed1, 0, 500  },   // store blinkLed1
  { printTick, 0, 2000 },   // store printTick
};
```

And the scheduler calls back into whichever function is due — without knowing or caring
what it does:

```cpp
void update() {
  for (int i = 0; i < count; i++) {
    Task *task = tasks + i;
    if (millis() - task->timer >= task->interval) {
      task->action();              // <-- the callback fires here
      task->timer += task->interval;
    }
  }
}
```

That `task->action()` is the payoff: one timing engine, any behavior.

---

## 6. Arrays of function pointers = jump tables

Because callbacks are data, you can index them. This is a classic embedded pattern for
command dispatch and state machines (replaces a big `switch`):

```cpp
typedef void (*Handler)();

void cmdStart() { /* ... */ }
void cmdStop()  { /* ... */ }
void cmdReset() { /* ... */ }

Handler handlers[] = { cmdStart, cmdStop, cmdReset };

handlers[cmd]();   // O(1) dispatch instead of if/else chains
```

---

## 7. The limitation that leads to lambdas

A plain function pointer can point only at a **standalone** function. It **cannot
carry data**. This is why all three of your callbacks had to reference *global*
objects (`led1`, `led2`):

```cpp
void blinkLed1() { led1.toggle(); }   // works only because led1 is global
```

You might wish you could write *one* parameterized callback:

```cpp
void blink(Led& led) { led.toggle(); }   // can't store this as a void(*)() — wrong shape
```

A **non-capturing** lambda still converts to a function pointer:

```cpp
TaskCallback cb = []() { Serial.println("ok"); };   // OK — captures nothing
```

But a **capturing** lambda (one that remembers a variable) does **not**:

```cpp
int pin = 13;
TaskCallback cb = [pin]() { digitalWrite(pin, HIGH); };   // ERROR: not a plain pointer
```

To carry state you need one of:
- a `void* context` passed alongside the function pointer (the classic C trick), or
- `std::function` (flexible, but heavy — generally avoided on small AVRs), or
- member-function pointers.

That trade-off is the bridge to a future lesson on lambdas and `std::function`.

---

## 8. Embedded relevance

Function pointers are everywhere in firmware:

- **Interrupts.** `attachInterrupt(digitalPinToInterrupt(2), myISR, RISING)` hands the
  hardware a function pointer (`myISR`) to call when the pin changes. The chip's
  interrupt vector table *is* an array of function pointers.
- **Drivers / libraries.** Callbacks let a library notify your code ("data ready",
  "timer elapsed") without the library depending on your code.
- **State machines.** A "current state" can be a function pointer; transitioning means
  reassigning it.

On AVR specifically: a function pointer is **2 bytes** and addresses **flash** (program
memory), since AVR keeps code and data in separate address spaces.

---

## 9. Gotchas cheat-sheet

| You write | Meaning |
|---|---|
| `f` | the function's address (pass this as a callback) |
| `f()` | call the function now |
| `&f` | also the function's address — identical to `f` for functions |
| `cb()` | call through the pointer |
| `(*cb)()` | call through the pointer — identical to `cb()` |

- The pointer's type must match the function's signature **exactly** (return type + args).
- Storing `f()` instead of `f` in a `Task{}` is the #1 beginner mistake — that calls the
  function immediately and tries to store its (void) result.

---

## TL;DR

A function pointer lets you treat **behavior as a value**. Your scheduler stores that
value (`TaskCallback action`) and calls it when due — so *when to run* (the scheduler)
and *what to run* (the callback) become independent. That separation is the foundation
of interrupts, drivers, and event-driven firmware.
