# Generic Components: Passing Data Into a Class (Challenge #21)

A companion to `21_generic_task_scheduler.ino`. This challenge promotes the loop you
wrote in #19 into a **reusable class** — a `Scheduler` that runs *whatever* task array you
hand it, without knowing at compile time how many tasks exist. That "give me your data, I
don't care how much" design is how real embedded frameworks are built. Read this next to
the sketch's header comment, which is the spec we're implementing.

---

## 1. The goal: a component, not a one-off loop

In #19 the scheduler *was* the `for` loop sitting in `loop()`, hardcoded to one `tasks[]`
array. That works once, but it isn't reusable — you can't drop it into another project, or
run two independent schedulers, without copy-pasting.

The fix is to wrap the timing engine in a class and **inject the data from outside**:

```cpp
Scheduler scheduler(tasks, size);   // hand the array + count to the component

void loop() {
  scheduler.update();               // the component does the work
}
```

The data (`tasks`, `size`) lives in *your* sketch; the *logic* lives in `Scheduler`. They
meet through the constructor.

---

## 2. The data: same `struct Task` as #19

```cpp
struct Task {
  Led led;
  unsigned long timer;
  unsigned long interval;
};
```

Nothing new here — this is the array-of-structs from Challenge #19. What changes is *who*
loops over it.

---

## 3. The class owns a pointer + a count

This is the heart of the challenge. The `Scheduler` stores **a pointer to your array** and
**how many elements it has**:

```cpp
class Scheduler {
private:
  Task* tasks;        // WHERE your task array lives (an address)
  uint8_t taskCount;  // HOW MANY tasks are in it
public:
  Scheduler(Task* t, uint8_t count) {
    tasks = t;
    taskCount = count;
  }
  void update();
};
```

Notice what `Scheduler` does **not** do: it does not store the tasks *themselves*, and it
does not bake in the number `3`. It holds only an address and a count. That's the same
"pointer + count" pair you met in #20 — now remembered by an object instead of passed on
every call.

```
your sketch:                       the Scheduler object:

tasks[]: [Task][Task][Task]        ┌─────────────────────┐
         ▲                         │ tasks    ●──────────┼──► points at tasks[0]
         └─────────────────────────┤ taskCount = 3       │
                                   └─────────────────────┘
       (the real data)                (just an address + a number)
```

Because `Scheduler` only holds an address, it can manage an array of **any size**, decided
by *you* at construction — exactly the "must not know how many tasks at compile time" rule.

---

## 4. Why a pointer, and where array decay sneaks back in

When you build the scheduler:

```cpp
Task tasks[] = {{8, 0, 500}, {9, 0, 1000}, {10, 0, 2000}};
int size = sizeof(tasks) / sizeof(tasks[0]);   // 3 — measured HERE, while tasks is a real array
Scheduler scheduler(tasks, size);
```

`tasks` **decays** to `Task*` as it's passed into the constructor — same mechanic as
Challenge #20. The constructor receives the address of `tasks[0]`, stores it, and that's
all the scheduler ever knows about your array.

This is why you compute `size` in the sketch (where `tasks` is still a true array and
`sizeof` works) and pass it in. Inside `Scheduler`, `sizeof(tasks)` would just measure the
pointer (2 bytes), not the array — the count *must* be carried explicitly.

> **Lifetime caveat:** the scheduler only *borrows* your array; it doesn't own a copy. The
> real `tasks[]` must stay alive as long as the scheduler uses it. Here it's a global, so
> it lives forever — fine. (Hand a class a pointer to a local array that goes out of scope
> and you get a dangling pointer.)

---

## 5. The engine: iterate through the pointer

`update()` is the #19 loop, but driven through the stored pointer and count:

```cpp
void Scheduler::update() {
  for (uint8_t i = 0; i < taskCount; i++) {
    Task* task = &tasks[i];                 // address of element i  (or: tasks + i)
    if (millis() - task->timer >= task->interval) {
      task->led.toggle();
      task->timer += task->interval;
    }
  }
}
```

Two pointer details:

- `&tasks[i]` (equivalently `tasks + i`) is the address of the i-th task. `tasks[i]` and
  `*(tasks + i)` are the same element — indexing *is* pointer arithmetic.
- `task->timer` is the arrow operator: `task->timer` means `(*task).timer` — "dereference
  the pointer, then take the field." You use `->` with pointers and `.` with values /
  references.

Because `task` points at the **real** array element, `task->timer += ...` updates *your*
`tasks[]` in place — the schedule actually advances.

---

## 6. Composition: a class made of other classes

`Scheduler` is **composed of** the parts it needs: a `Task*` and a count; each `Task` is in
turn composed of an `Led`, a timer, and an interval. This layering — objects holding
objects — is **composition** ("has-a"):

```
Scheduler ──has──► Task* ──points to──► Task ──has──► Led
                                         └──has──► timer, interval
```

Composition is how you build big systems from small, independently-understandable pieces.
(Challenge #23 will introduce the *other* relationship — inheritance, "is-a".)

---

## 7. Embedded angle: this is the framework pattern

"User provides a data array + count; a library object operates on it" is *everywhere* in
embedded:

- A display library takes a pointer to your framebuffer + its dimensions.
- A menu library takes a pointer to your array of menu entries + how many.
- An RTOS takes a pointer to your task descriptors.

The library ships as one fixed binary; **you** supply the data that specializes it. That
keeps the component reusable across projects and keeps flash usage down (one copy of the
engine, no matter how many tasks). You could even create two `Scheduler` objects over two
different arrays — impossible with the hardcoded `loop()` from #19.

---

## 8. The bridge to Challenges 22 & 23

Right now every task does the same thing: `task->led.toggle()`. The engine is reusable, but
the *behavior* is fixed. The next challenges make the behavior pluggable too:

- **#22** replaces the `Led led` field with a **function pointer** (`TaskCallback action`),
  so a task can run any function — the scheduler calls `task->action()` instead of
  `task->led.toggle()`.
- **#23** turns `Task` into a base class with a `virtual run()`, so the scheduler holds
  `Task**` (pointers to `BlinkTask` / `PrintTask` objects) and each task defines its own
  behavior.

Both reuse the exact skeleton you build here — a class that owns a pointer to user data and
loops over it. You've built the chassis; #22 and #23 swap the engine.

---

## Gotchas

| Symptom | Cause |
|---|---|
| `sizeof` inside `Scheduler` gives 2 (a pointer size) | The array decayed to `Task*`; compute `size` in the sketch and pass it in |
| Scheduler runs garbage / crashes | Stored pointer outlived the array (dangling), or `taskCount` doesn't match the real length |
| `task.timer` won't compile | `task` is a `Task*` — use `task->timer`, not `task.timer` |
| Timers never advance | Looped over a copy instead of `&tasks[i]`; update through the pointer so it hits the real element |
| Wants to "store more tasks later" | A raw `Task*` + count is a borrowed, fixed-size view — it doesn't grow itself |

---

## TL;DR

A generic component **owns a pointer to your data plus a count**, not the data itself.
`Scheduler` stores `Task* tasks` and `uint8_t taskCount` (handed in via its constructor,
where `tasks` decays to `Task*`), then loops `tasks[i]` through `task->...` to drive your
real array. The engine never hardcodes how many tasks exist — *you* decide at construction.
That "pass data + count into a reusable class" pattern is the foundation of embedded
frameworks, and the chassis the next two challenges build on.
