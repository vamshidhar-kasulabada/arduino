# Cooperative Scheduling & Multiple Timers (Challenge #18)

A companion to `18_multi_timer_scheduler.ino`. Three LEDs blink at three different rates
— 500 ms, 1000 ms, 2000 ms — all at once, with no `delay()` anywhere. This note explains
how an embedded system runs many periodic tasks "at the same time" without an operating
system, tied to the code you wrote.

---

## 1. Why `delay()` can't do this

`delay(500)` freezes the *entire* CPU for 500 ms. With three LEDs at three rates, the
moment you `delay()` for one, the other two are stuck. There's only one CPU, so blocking
it for any task blocks *all* tasks:

```
delay(500); led1.toggle();   // for 500ms, led2 and led3 are FROZEN
```

The fix isn't real parallelism — it's making each task **non-blocking**: do a tiny bit of
work, then immediately let the others have a turn.

---

## 2. The millis() timer pattern

The building block is a single non-blocking timer. Instead of *waiting*, you *remember
when you last acted* and *check the clock*:

```cpp
if (millis() - timer >= interval) {   // has 'interval' ms passed?
  led.toggle();
  timer += interval;                  // mark this slot done
}
```

`millis()` is the milliseconds-since-boot clock that keeps ticking on its own.
`millis() - timer` is "time elapsed since I last fired." When that reaches `interval`,
fire and advance `timer`. The check is *instant* — if the interval hasn't passed, the
`if` is false and we move on. No blocking.

Notice it advances `timer += interval`, **not** `timer = millis()`. Adding the fixed
interval keeps the schedule anchored to the original grid, so timing doesn't drift even
if a loop is slightly late.

---

## 3. One timer per task

The key rule from the challenge: *each LED must have its own timer*. Three LEDs need three
independent `(led, timer, interval)` triples. The naive version (shown commented-out in
the sketch) spells them out:

```cpp
unsigned long timer1 = 0, timer2 = 0, timer3 = 0;
// three near-identical if-blocks ...
```

Because each task carries its own `timer` and `interval`, they don't interfere — LED1
hitting 500 ms says nothing about where LED2 is in its 1000 ms cycle.

---

## 4. Cooperative scheduling: how `loop()` interleaves

Every pass through `loop()` checks *all three* timers, fires whichever are due, and moves
on. No task ever hogs the CPU — each "cooperates" by returning quickly. That's
**cooperative scheduling**.

```
loop pass:  check t1? check t2? check t3?  -> repeat thousands of times/sec
                |         |         |
            fire when  fire when  fire when
             due        due        due
```

Because `loop()` runs far faster than the intervals (microseconds vs. hundreds of ms),
from the outside it *looks* like three tasks running concurrently. This is how
microcontrollers do "many things at once" with no operating system — there's no real
threading, just one fast loop politely visiting each task.

---

## 5. From repeated code to data: the array refactor

Three copy-pasted `if` blocks beg to be collapsed. The working sketch turns the *tasks*
into **parallel arrays** — the data — and the *logic* into one loop:

```cpp
Led leds[] = {8, 9, 10};
const uint8_t size = sizeof(leds) / sizeof(leds[0]);
unsigned long timers[]    = {0, 0, 0};
unsigned long intervals[] = {500, 1000, 2000};
```

Index `i` ties the three arrays together: `leds[i]` is driven by `timers[i]` at
`intervals[i]`.

```
 index:      0        1         2
 leds:    Led(8)   Led(9)    Led(10)
 timers:    0        0         0
 intervals: 500     1000      2000
            \_______ one task per column _______/
```

Two C++ details worth noting:

- `Led leds[] = {8, 9, 10};` — those ints become `Led` objects because the `Led`
  constructor takes a single `uint8_t` (implicit conversion). Each element's constructor
  runs `pinMode(pin, OUTPUT)`, so the pins are configured automatically.
- `sizeof(leds) / sizeof(leds[0])` — the idiom for array length: total bytes divided by
  one element's bytes. Computing `size` this way means adding a 4th LED requires editing
  only the array literals, not the loop.

---

## 6. The whole scheduler in one loop

All three timers, handled by a single body:

```cpp
void loop() {
  for (int i = 0; i < size; i++) {
    if (millis() - timers[i] >= intervals[i]) {
      leds[i].toggle();
      timers[i] += intervals[i];
    }
  }
}
```

This *is* a tiny scheduler. Want ten blinking LEDs at ten rates? Add ten entries to the
arrays — the loop doesn't change. (This is the same engine that, in challenge #22, grows
to store a *function pointer* per task instead of just an LED — so each slot can run
arbitrary behaviour, not only `toggle()`.)

---

## 7. Independence on the timeline

Because each task keeps its own clock, the schedules overlap cleanly — matching the table
in the sketch header:

```
 t(ms): 0    500   1000  1500  2000
 LED1:  -    tog   tog   tog   tog     (every 500)
 LED2:  -     -    tog    -    tog     (every 1000)
 LED3:  -     -     -     -    tog     (every 2000)
```

At 2000 ms all three happen to fire in the same loop pass — and that's fine. They run
back-to-back in microseconds; none waits on the others.

---

## Gotchas

| Pitfall | Why it bites | Fix |
|---|---|---|
| Using `delay()` for any timer | Blocks every other task | Use `millis() - timer >= interval` |
| `timer = millis()` instead of `timer += interval` | Schedule drifts over time | Add the fixed interval |
| Long-running work inside the loop | Stalls all other tasks (cooperative = trust-based) | Keep each task tiny and non-blocking |
| Hard-coding array length | Easy to desync when you add a task | `sizeof(arr)/sizeof(arr[0])` |
| Mismatched array lengths | `timers`/`intervals`/`leds` get out of sync | Keep the parallel arrays the same size |
| `==` instead of `>=` on the check | A skipped tick misses the exact match forever | Always use `>=` |

---

## TL;DR

Three LEDs blink at three rates because each is a **non-blocking `millis()` timer** with
its own `timer` and `interval`, and `loop()` checks them all every pass — firing whichever
are due and returning instantly. That's **cooperative scheduling**: one fast loop visits
each task in turn, so the system *appears* concurrent with no OS and no threads. The array
refactor (`leds[]`, `timers[]`, `intervals[]` indexed by `i`) collapses three copies of
the same logic into one loop that scales to any number of tasks.
