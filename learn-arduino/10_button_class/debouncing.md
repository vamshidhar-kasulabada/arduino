# Switch Debouncing & the Button Class (Challenge #10)

A companion to `10_button_class.ino`. The `Button` class in this challenge reports a
*clean* press from a *dirty* mechanical signal. This note explains **why the signal is
dirty (bounce)**, how `wasPressed()` cleans it up with `millis()`, and how the whole
thing is wrapped in a class so application code never touches `digitalRead()`.

---

## 1. The hardware problem: switch bounce

A push button is two metal contacts. When you press it, the contacts don't meet
cleanly — they physically **bounce** apart and together a few times over a few
milliseconds before settling. To the microcontroller, one human press looks like a
burst of rapid HIGH/LOW transitions:

```
       press                                 settled
         |                                      |
 HIGH ───┐ ┌─┐ ┌──┐                             ┌──────────────
         │ │ │ │  │                             │   (button released)
 LOW     └─┘ └─┘  └─────────────────────────────┘
         <-- bounce -->        stable LOW (held)
         ~1-10 ms
```

If you naively count every HIGH→LOW edge as a press, one physical tap registers as
3, 5, or 8 presses. **Debouncing** is the act of ignoring the noisy edges and reporting
only the one real transition.

---

## 2. Why the pin reads HIGH when idle (`INPUT_PULLUP`)

The constructor configures the pin like this:

```cpp
Button(uint8_t p)
    : pin(p), stableState(HIGH), lastReading(HIGH), lastChangeTime(0) {
  pinMode(pin, INPUT_PULLUP);
}
```

`INPUT_PULLUP` turns on a resistor *inside* the chip that gently pulls the pin up to
5V. So the wiring is:

```
   5V ──[ internal pull-up ]── pin 2 ──[ button ]── GND
```

- **Not pressed:** the pull-up wins, pin reads **HIGH**.
- **Pressed:** the button shorts the pin straight to GND, pin reads **LOW**.

This is why the logic is "active-low": a press is `HIGH → LOW`. It also means you need
**no external resistor** — a cleaner circuit. Notice `stableState` and `lastReading`
are both seeded to `HIGH` so the very first `loop()` doesn't see a phantom transition.

---

## 3. Non-blocking debounce with `millis()`

The naive textbook fix is `delay(20)` after seeing a change — but `delay()` freezes the
whole sketch. This class uses the **stable-state** technique instead: keep reading, and
only trust a value once it has held steady for 20 ms.

```cpp
bool wasPressed() {
  bool currentReading = digitalRead(pin);

  if (currentReading != lastReading) {   // any edge — even bounce — resets the clock
    lastChangeTime = millis();
    lastReading = currentReading;
  }

  if (millis() - lastChangeTime >= 20) { // signal has been quiet for 20 ms
    if (stableState != currentReading) { // and it differs from our committed value
      stableState = currentReading;
      if (stableState == LOW) {          // committed HIGH -> LOW == a real press
        return true;
      }
    }
  }
  return false;
}
```

The key insight: **every bounce edge restarts the 20 ms timer.** So during the noisy
burst, `millis() - lastChangeTime` keeps getting reset and never reaches 20. Only once
the contacts settle and stay put for a full 20 ms does the timer expire — and only
*then* does `stableState` update. The bounce is filtered out by construction.

There are two distinct "states" tracked here, and the distinction matters:

| Variable | Meaning |
|---|---|
| `lastReading` | the **raw** value seen on the previous call (changes on every bounce) |
| `stableState` | the **committed** value we believe is real (changes only after 20 ms quiet) |
| `lastChangeTime` | `millis()` timestamp of the last *raw* change — the debounce clock |

---

## 4. Walking the timeline

```
 raw pin:   H H L H L L L L L L L L L L  ...   (bounce, then settles LOW)
 call #:    1 2 3 4 5 6 7 8 9 ...
                ^   ^   ^               ^
 t=lastChange   reset reset reset       (no more changes)
                                        |
                          20 ms after the LAST edge -> stableState := LOW -> return true ONCE
```

Every change in the bounce burst pushes `lastChangeTime` forward. The `>= 20` check
only succeeds well after the chatter stops, so `wasPressed()` returns `true` exactly
**one time** per physical press — which is the whole goal.

`millis() - lastChangeTime` is **unsigned subtraction**, so it stays correct even when
`millis()` rolls over (~49 days). Never compare timestamps with `<` directly; always
subtract then compare — a pattern you'll reuse in every non-blocking sketch.

---

## 5. The C++ angle: encapsulation & state management

All the messy details — the pin number, the three state variables, the 20 ms rule —
are `private`. The application only sees one verb:

```cpp
Button button(2);

void loop() {
  if (button.wasPressed()) {
    redLed.toggle();
  }
}
```

This is **encapsulation**: the noisy hardware reality is hidden behind a clean,
intention-revealing API. The rules for this challenge make the point explicit —
application code must *never* call `digitalRead()` and must *never* implement debounce
itself. Those responsibilities live inside the object.

`wasPressed()` is also an **event detector**, not a state query. It does not answer "is
the button down right now?" — it answers "did a fresh press *just* happen?" and it can
only ever fire once per press because it mutates `stableState` as a side effect. This
edge-vs-level distinction is fundamental in firmware: you usually want to act on the
*transition*, not the *level*.

---

## 6. Why a class beats a pile of globals

Each `Button` carries its own `pin`, `stableState`, `lastReading`, and `lastChangeTime`.
Want a second button? Just declare `Button start(2), reset(3);` — each instance debounces
independently with zero copy-pasted timing code. Without the class you'd need a parallel
set of `lastChangeTime_2`, `lastReading_2`, … globals for every button, which doesn't
scale. The class **bundles state with the behavior that owns it**.

```
   Button button         Button start          Button reset
  ┌──────────────┐      ┌──────────────┐      ┌──────────────┐
  │ pin           = 2 │  │ pin           = 2 │  │ pin           = 3 │
  │ stableState   = H │  │ stableState   = H │  │ stableState   = H │
  │ lastReading   = H │  │ lastReading   = H │  │ lastReading   = H │
  │ lastChangeTime    │  │ lastChangeTime    │  │ lastChangeTime    │
  └──────────────┘      └──────────────┘      └──────────────┘
        (each object is a self-contained debouncer)
```

---

## 7. Embedded relevance

- **`digitalRead(pin)` returns a `byte`** (0 or 1), here stored in a `bool` — fine,
  because `HIGH`/`LOW` are `1`/`0`. The comparison `stableState == LOW` is just `== 0`.
- **Debounce time is a tuning knob.** 20 ms suits most tactile buttons; cheap or worn
  switches may need 50 ms. Too long and fast presses feel laggy; too short and bounce
  leaks through.
- **Polling vs. interrupts.** This class polls inside `loop()`. You *can* debounce in a
  pin-change interrupt instead, but debouncing in an ISR is tricky (you can't easily
  read `millis()` reliably while time-sensitive), so polling is the common beginner-safe
  choice.
- **No `delay()` anywhere.** Because `wasPressed()` returns immediately, `loop()` stays
  free to do other work — the foundation for running many tasks "at once."

---

## Gotchas

| Pitfall | What happens | Fix |
|---|---|---|
| Using `delay(20)` to debounce | Freezes the whole sketch; can't read other inputs | Use the `millis()` stable-state pattern |
| Counting raw `digitalRead()` edges | One press counts as 3–8 presses | Commit a value only after 20 ms of quiet |
| Forgetting `INPUT_PULLUP` | Pin floats; random HIGH/LOW noise | Use internal pull-up (or external resistor) |
| Comparing `millis() < lastChangeTime + 20` | Breaks on 49-day rollover | Subtract first: `millis() - lastChangeTime >= 20` |
| Treating `wasPressed()` as "is it held?" | Misuse — it fires once per press, not continuously | Add a separate `isDown()` if you need the level |
| Seeding `stableState`/`lastReading` to `LOW` | Phantom press on first loop | Seed to the idle value, `HIGH` |

---

## TL;DR

A mechanical button **bounces**: one press produces a burst of fake HIGH/LOW edges. The
`Button` class hides a `millis()`-based **stable-state debouncer** — every edge resets a
20 ms timer, and a value is only *committed* once it holds steady that long. Wrapped in a
class, all of this (pin, `INPUT_PULLUP`, timing, state) is `private`, so `loop()` just asks
`button.wasPressed()` and gets exactly one clean event per real press. That is encapsulation
and non-blocking event detection working together.
