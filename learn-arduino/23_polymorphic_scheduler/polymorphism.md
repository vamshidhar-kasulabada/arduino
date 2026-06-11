# Inheritance, Virtual Functions & Polymorphism (Challenge #23)

A companion to `23_polymorphic_scheduler.ino`. This is the concept that didn't click on
first contact — that's normal. Read it next to your code; everything here uses your
actual `Task` / `BlinkTask` / `PrintTask` / `Scheduler`.

---

## 1. The one-sentence idea

**Polymorphism = one interface, many behaviours.** The scheduler holds things that all
*look* like a `Task` (they all have `run()`), but each one *does* something different.
The scheduler calls `run()` and the object itself decides what actually happens.

---

## 2. Inheritance: building on a base class

```cpp
class BlinkTask : public Task { ... };
//                ^^^^^^^^^^^^
//                "a BlinkTask IS-A Task, and gets everything Task has"
```

`BlinkTask` inherits `Task`'s `timer`, `interval`, and `update()`. It *adds* its own
`Led led;` and *provides* its own `run()`. So a `BlinkTask` is a `Task` plus extra.

The constructor passes the shared part up to the base:

```cpp
BlinkTask(uint8_t pin, unsigned long interval)
  : Task(interval),   // build the Task part first (sets timer/interval)
    led(pin) {}       // then build our own member
```

---

## 3. `virtual`: decide at *runtime*, not compile time

A normal function call is decided when you compile. A **virtual** function is decided
when the program *runs*, based on what the object really is:

```cpp
virtual void run() = 0;   // in Task
void run() override;      // in BlinkTask / PrintTask
```

When you call `someTask->run()`, the program looks at the *actual object* behind the
pointer and runs *that* class's `run()`. That late decision is the whole magic — and
it's why this is called **dynamic dispatch**.

`override` isn't required, but it tells the compiler "I mean to replace a base virtual" —
if the signature doesn't match a base method, you get an error instead of silently
creating a new unrelated function. Always use it.

---

## 4. Pure virtual (`= 0`) and abstract classes

```cpp
virtual void run() = 0;   // pure virtual: NO body in the base
```

`= 0` means "the base refuses to define this — every subclass must." A class with a pure
virtual is **abstract**: you cannot create one.

```cpp
Task t(500);        // ERROR: Task is abstract
BlinkTask b(9, 500); // OK: BlinkTask defines run(), so it's concrete
```

This is exactly why an *array of `Task` values* is impossible — you can't store something
you can't even create. Which leads to the part that confused you...

---

## 5. Why pointers? (object slicing — the core gotcha)

Your first attempt stored tasks **by value** (`Task* _tasks` indexed as `_tasks[i]`,
giving `Task` objects). Here is what goes wrong, in memory.

A `Task*` array tries to pack whole objects side by side, each `sizeof(Task)` big:

```
Task* tasks[]:   [  Task  ][  Task  ][  Task  ]
                    ^ but a BlinkTask is BIGGER (it has an Led).
                      The extra part gets chopped off  ->  SLICING.
```

The `Led` and the vtable info get sliced away, so `run()` can't find `BlinkTask::run()`.
(And `Task` is abstract, so this can't even compile.)

The fix is an array of **pointers** to the real objects, which live elsewhere:

```
Task* tasks[]:   [ Task* ][ Task* ][ Task* ]    <- just addresses, all same size
                    │         │         │
                    ▼         ▼         ▼
                BlinkTask  BlinkTask  PrintTask  <- full objects, any size, intact
```

Each pointer can aim at a different subclass of a different size, because a pointer is
always the same size. Calling `tasks[i]->run()` follows the address to the *intact*
object, so dynamic dispatch finds the right `run()`.

> **Rule:** polymorphism only works through a **pointer or reference** to the base.
> A base *value* slices the object and kills it.

And recall the chain of types:
- `Task* tasks[] = { &blink1, ... };` → `tasks` is an **array of `Task*`**.
- Passed to a function it decays to `Task**` → that's why `Scheduler` stores `Task**`.

---

## 6. How it works under the hood (the vtable)

Each class with virtual functions gets one hidden table of function addresses — the
**vtable** — listing where its `run()`, destructor, etc. live. Every *object* carries one
hidden pointer to its class's vtable.

```
blink1 ─► [vptr]──► BlinkTask vtable ─► BlinkTask::run
print  ─► [vptr]──► PrintTask vtable ─► PrintTask::run
```

`task->run()` really means: "follow the object's vptr to its vtable, look up `run`, call
it." That's how one line of code (`task->run()`) runs different functions. On AVR that
hidden vptr costs **2 bytes of RAM per object** — the price of the flexibility.

---

## 7. Walk through one call in your code

```cpp
Task* tasks[] = { &blink1, &blink2, &printer };
scheduler.update();
```

1. `Scheduler::update()` loops and calls `tasks[i]->update()`.
2. `update()` lives in the base (not virtual) — it does the timing check.
3. When due, `update()` calls `run()`. `run()` **is** virtual.
4. For `&blink1`, the vptr points at `BlinkTask`'s vtable → `BlinkTask::run()` →
   `led.toggle()`. For `&printer` → `PrintTask::run()` → `Serial.println(msg)`.

One `update()`, one `run()` call site, three different behaviours. The scheduler never
mentions `Led` or `Serial`.

---

## 8. Template Method pattern (you used it without naming it)

Splitting `update()` (non-virtual, shared "WHEN") from `run()` (virtual, per-subclass
"WHAT") is a classic pattern: the base controls the *skeleton* of the operation and lets
subclasses fill in the *steps*. The base calling a virtual is the heart of it.

---

## 9. Virtual functions vs function pointers (Challenge 22 vs 23)

Both decouple *when* from *what*. The difference is **data**:

| | Challenge 22 (function pointers) | Challenge 23 (virtual functions) |
|---|---|---|
| The "what" is | a free function | an object's `run()` |
| Carries its own data? | No — needed globals | **Yes** — each object owns its members |
| Uniform interface | `void()` signature | base class `Task` |
| Cost | 2-byte code pointer | 2-byte vptr per object |

Challenge 23 is the answer to 22's wall: a `BlinkTask` *is* a callback that remembers its
own `Led`.

---

## 10. Virtual destructor — why the base has `virtual ~Task()`

If you ever `delete` an object through a `Task*`, a **non**-virtual destructor would only
run `~Task()` and leak the subclass's parts. A virtual destructor routes to the real
subclass destructor first. Rule of thumb: **any class with a virtual function should have
a virtual destructor.** (You're using stack/global objects here so it doesn't bite, but
it's the correct habit.)

---

## 11. Gotchas cheat-sheet

| Symptom | Cause |
|---|---|
| Base version runs instead of subclass | called through a **value**, not a pointer/reference (slicing), or method wasn't `virtual` |
| "cannot declare variable of abstract type" | tried to create / store-by-value a class with a pure virtual |
| `override` compile error | your signature doesn't match the base's — a real bug it just caught |
| subclass data is garbage | object was sliced into a base value somewhere |

---

## TL;DR

`virtual` = "decide which function at runtime, from the real object." That decision needs
the **real object**, so you reach it through a **pointer/reference** to the base — a base
*value* slices it. Your `Scheduler` holds `Task**` (addresses of real `BlinkTask`/
`PrintTask` objects) and calls `->run()`, and each object runs its own version. One
interface, many behaviours.
