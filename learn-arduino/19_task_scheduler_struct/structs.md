# Structs & Arrays-of-Structs (Challenge #19)

A companion to `19_task_scheduler_struct.ino`. This challenge looks like a small
refactor — three blinking LEDs — but it teaches the single most important idea in data
modeling: **group related data so it travels together.** Read this next to your code;
everything uses your actual `Task` struct and `tasks[]` array.

---

## 1. The problem we're fixing: parallel arrays

Before this challenge, a multi-timer scheduler kept three *separate* arrays, one per
field, all indexed by the same `i`:

```cpp
Led          leds[]      = { 8, 9, 10 };
unsigned long timers[]    = { 0, 0, 0  };
unsigned long intervals[] = { 500, 1000, 2000 };
```

These are **parallel arrays**: `leds[i]`, `timers[i]`, `intervals[i]` together describe
*one* task, but the language doesn't know they belong together. The relationship lives
only in your head (and in the matching index).

That's fragile:

- Add a 4th LED and forget to grow one array → silent mismatch.
- Sort or reorder one array → all three desync.
- Pass "a task" to a function → you must pass three arguments and hope they line up.

The rule the challenge enforces is exactly this: **do not use separate `leds[]`,
`timers[]`, `intervals[]`. Use a single array of structs.**

---

## 2. A `struct` glues fields into one type

A `struct` is a new data type you build by bundling existing types together. This is
your sketch's definition:

```cpp
struct Task {
  Led led;                // WHICH LED
  unsigned long timer;    // WHEN it last fired (a millis() timestamp)
  unsigned long interval; // how often (ms)
};
```

Now `Task` *is* a type, just like `int` or `Led`. One `Task` value carries all three
fields as a single unit. The desync problem disappears because there is nothing to keep
in sync — the fields are physically welded together.

Accessing a field uses the dot operator:

```cpp
Task t = {8, 0, 500};
t.interval;     // 500
t.led.toggle(); // reach into the Led member, then call its method
```

---

## 3. How a struct sits in memory

A struct lays its members out **in order, back to back** (the compiler may insert
*padding* so each member lands on an address its type likes). Conceptually one `Task`:

```
        ┌─────────── one Task ───────────┐
        │  led        timer     interval │
        │ [Led]   [unsigned long][u.long] │
        │  1 byte*    4 bytes      4 bytes │   (*Led holds a uint8_t pin + bool state)
        └────────────────────────────────┘
```

`sizeof(Task)` is roughly the sum of the members (plus padding). The key point: the
fields of one task are **contiguous** — they are one chunk of memory with one identity.

---

## 4. Array-of-structs: the layout that matters

Your sketch then makes an **array of `Task`**:

```cpp
Task tasks[] = {{8, 0, 500}, {9, 0, 1000}, {10, 0, 2000}};
```

Each inner `{...}` is one `Task`. In RAM the whole array is one run of `Task`-sized
blocks, each block self-contained:

```
tasks[]:  ┌── tasks[0] ──┐┌── tasks[1] ──┐┌── tasks[2] ──┐
          │ led  t  intv ││ led  t  intv ││ led  t  intv │
          │  8   0  500  ││  9   0  1000 ││ 10   0  2000 │
          └──────────────┘└──────────────┘└──────────────┘
           one Task        one Task        one Task
```

Compare that to the *parallel arrays* layout, where one task is scattered across three
different regions of memory:

```
leds[]:      [ 8 ][ 9 ][ 10 ]      ← a task's pieces
timers[]:    [ 0 ][ 0 ][ 0  ]      ←   are spread
intervals[]: [500][1000][2000]     ←   across 3 arrays
```

Array-of-structs keeps each task's data **local** — which is also friendlier to the
hardware (a single task's fields are near each other in memory).

---

## 5. Aggregate initialization (`{...}`)

`{{8, 0, 500}, ...}` is **aggregate initialization**: the values fill the members
top-to-bottom in declaration order.

```cpp
{8, 0, 500}
//│  │   └── interval
//│  └────── timer
//└───────── led   ← initializes the Led member from the pin 8
```

That first `8` runs `Led`'s constructor `Led(uint8_t p)`, which calls `pinMode(pin,
OUTPUT)`. So building the array also configures the hardware. **Order is everything** —
swap two values and you'd quietly assign the interval to the timer.

---

## 6. Walking the array (your scheduler loop)

The scheduler is now beautifully uniform — one loop, one variable per task:

```cpp
for (Task &task : tasks) {
  if (millis() - task.timer >= task.interval) {
    task.led.toggle();
    task.timer += task.interval;
  }
}
```

Two things worth naming:

- `Task &task` is a **reference** — `task` is an *alias* for the real element in
  `tasks[]`, not a copy. That matters on the next line: `task.timer += ...` updates the
  **actual** array element. Drop the `&` (`for (Task task : tasks)`) and you'd update a
  throwaway copy, so the timer would never advance and the LED would never toggle.
- `task.timer += task.interval` (instead of `= millis()`) keeps the schedule on a fixed
  grid and absorbs small loop delays — drift-free periodic timing.

---

## 7. Embedded angle: this is how real firmware scales

"Array of structs + loop over it" is the backbone of cooperative task scheduling. To
add a 4th LED you add **one row** — `{11, 0, 250}` — and the loop already handles it. No
new arrays, no new `if` blocks. The data describes the system; the code stays fixed.

This is *data-driven design*: behavior is configured by a table, not hardcoded. Device
descriptor tables, pin-mapping tables, and command tables in production firmware all use
this exact shape.

---

## 8. The bridge to Challenges 20, 21, 22, 23

You've just learned to model "a thing" as a `struct` and store many of them in an array.
Everything after this builds on it:

- **#20** passes that array into a function — and you'll see the array "decay" to a
  pointer.
- **#21** hands the array (a `Task*` + a count) to a `Scheduler` *class*, making the
  engine reusable.
- **#22** swaps the `Led led` field for a *function pointer* so a task can do anything.
- **#23** turns `Task` into a base class so each task is its own object.

The `struct Task { ... timer; ... interval; }` you wrote here is the seed of all of it.

---

## Gotchas

| Symptom | Cause |
|---|---|
| Timer never advances / LED frozen | Looped with `Task task` (a copy) instead of `Task &task` (a reference) |
| Wrong field gets the value | Aggregate `{...}` values are positional — order must match the struct's member order |
| Pin never set to OUTPUT | The `Led led` member is only constructed when you give it a pin in the initializer |
| Forgot a comma between rows | `{{...} {...}}` won't compile — each `Task` row needs a separating `,` |
| Tasks "desync" after a change | You slipped back to parallel arrays; keep it one array of structs |

---

## TL;DR

A `struct` welds related fields (`Led led`, `timer`, `interval`) into one type so they
travel together; an **array of structs** (`Task tasks[]`) stores many tasks as
self-contained, contiguous blocks instead of three fragile parallel arrays. Loop over it
**by reference** (`for (Task &task : tasks)`) so updates hit the real elements. This is
data modeling — and the foundation every later scheduler challenge builds on.
