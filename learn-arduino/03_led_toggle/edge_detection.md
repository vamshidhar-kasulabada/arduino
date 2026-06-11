# Edge Detection & State Variables (Challenge #3)

A companion to `03_led_toggle.ino`. This challenge combines the output of #1 with the
input of #2, then adds the key new idea: reacting to the *moment* a button changes, not
its steady state. That trick is called **edge detection**, and it's the foundation of
almost every interactive embedded program.

---

## 1. The problem: a level vs. an event

You want one press to toggle the LED once. But `loop()` runs thousands of times per
second, and `digitalRead(8)` is `LOW` for the *entire time* you hold the button. So the
naive version misbehaves:

```cpp
// WRONG: toggles madly while you hold the button
if (digitalRead(8) == LOW) {
  ledOn = !ledOn;          // flips on every single loop iteration!
}
```

A press that lasts 100 ms might run this hundreds of times — the LED flickers randomly
and lands in an unpredictable state. The challenge's requirement says it plainly:
"Avoid repeated toggles while holding the button."

What you actually care about is the **transition** from released to pressed — a single
*event* — not the *level* of the pin.

---

## 2. State variables: remembering across iterations

`loop()` has no memory of the previous run — local variables vanish each time. To detect
a change you must *remember* what the pin was last time. That's what these two globals
are for:

```cpp
bool ledOn = false;            // the LED's logical state (does it stay on?)
int  previousPin8State = HIGH; // what pin 8 read on the PREVIOUS loop
```

- `ledOn` is the thing the user controls — it persists so the LED stays on/off between
  presses.
- `previousPin8State` is the memory that makes edge detection possible. It's initialised
  to `HIGH` because, with `INPUT_PULLUP` (see challenge #2), a released button reads HIGH
  — so we start assuming "not pressed".

Globals persist for the life of the program, which is exactly the kind of memory `loop()`
otherwise lacks.

---

## 3. The edge-detection condition

Each loop, read the pin *now*, then compare it to *last time*:

```cpp
void loop() {
  int currentPin8State = digitalRead(8);

  if (previousPin8State == HIGH && currentPin8State == LOW) {
    ledOn = !ledOn;                 // fire ONCE on the falling edge
  }

  previousPin8State = currentPin8State;   // remember for next time
  digitalWrite(13, ledOn);
}
```

The condition `previousPin8State == HIGH && currentPin8State == LOW` is true only on the
single iteration where the pin *just changed* from HIGH to LOW — the moment of pressing.
This is a **falling edge** (HIGH -> LOW). The next iteration, `previousPin8State` is also
LOW, so the condition is false until you release and press again.

```
 button:   released ──────press──────► held ──────► release
 pin 8:    HIGH HIGH HIGH | LOW  LOW  LOW  LOW | HIGH HIGH
                          ↑
                   falling edge: prev==HIGH && curr==LOW  -> toggle HERE (once)

 prev:     HIGH HIGH HIGH HIGH LOW  LOW  LOW  LOW  LOW  ...
 curr:     HIGH HIGH HIGH LOW  LOW  LOW  LOW  HIGH HIGH ...
 toggle?    no   no   no  YES  no   no   no   no   no
```

The two crucial lines work as a pair:
1. The `if` checks for the transition.
2. `previousPin8State = currentPin8State;` updates the memory **every** loop, so next
   iteration's comparison is correct. Forgetting this line breaks everything.

---

## 4. `ledOn = !ledOn` — flipping a boolean

```cpp
ledOn = !ledOn;   // true->false, false->true
```

`!` is logical NOT. This is the standard idiom for toggling a flag. Then:

```cpp
digitalWrite(13, ledOn);   // bool auto-converts: true->HIGH, false->LOW
```

Because `digitalWrite` takes an integer and `true`/`false` convert to `1`/`0`, you can
pass `ledOn` directly. Writing the pin every loop (not just on a change) is harmless —
it just re-asserts the current state — and keeps the code simple.

| `previousPin8State` | `currentPin8State` | Action |
|---|---|---|
| HIGH | HIGH | nothing (still released) |
| **HIGH** | **LOW** | **toggle `ledOn`** (falling edge / press) |
| LOW | LOW | nothing (still held) |
| LOW | HIGH | nothing (rising edge / release) |

---

## 5. The Button-Bounce discovery

This is the "Button Bounce Discovery" from the concepts list — and it's something you
*observe*, not something this sketch fixes. A real mechanical switch doesn't snap
cleanly from HIGH to LOW. Its metal contacts physically bounce for a few milliseconds,
producing several fast HIGH/LOW flickers:

```
 ideal:     HIGH ─────┐
                      └──────── LOW
 reality:   HIGH ──┐ ┌┐ ┌─┐
                   └─┘└─┘ └──── LOW   <- multiple edges in ~1-5 ms
```

Because each of those extra falling edges satisfies our `if`, a single physical press
can occasionally toggle the LED twice — so the LED sometimes lands in the "wrong" state.
That flaky behaviour is the intended lesson here: edge detection alone isn't enough for
mechanical inputs. The fix, **debouncing** (ignoring changes for a few ms using
`millis()`), is the subject of a later challenge.

---

## 6. The C++ / embedded angle

- `bool` vs `int`: `ledOn` is a `bool` (a logical on/off flag), while `previousPin8State`
  is an `int` to hold the `HIGH`/`LOW` constants `digitalRead` returns. Both work; the
  types document intent.
- **Edge detection is a 1-iteration state machine.** "Previous state + current state ->
  decide" generalises far beyond buttons: detecting a sensor crossing a threshold, a
  clock tick, or any "this just changed" event in firmware.
- This is the **polling** approach — you read the pin every loop. The hardware
  alternative is an **interrupt** (`attachInterrupt`) that fires automatically on an
  edge, freeing `loop()` from constant checking. Polling is simpler and fine here.
- Recall from #1/#2: pin 13 is OUTPUT (the LED), pin 8 is `INPUT_PULLUP` (the button,
  reading HIGH when released). That's why the edge we hunt for is HIGH -> LOW.

---

## Gotchas

| Mistake | What happens |
|---|---|
| Toggling on `digitalRead(8) == LOW` (level, not edge) | LED flips every loop while held — chaos |
| Forgetting `previousPin8State = currentPin8State;` | Memory never updates; edge logic breaks |
| Initialising `previousPin8State = LOW` | A "press" appears falsely on the first loop |
| Expecting clean single toggles | Switch **bounce** can double-toggle — needs debouncing |
| Making `previousPin8State` local to `loop()` | It resets each iteration; no memory, no edge detection |

---

## TL;DR

A held button is a *level*; a press is an *event*. To catch the event you remember the
pin's previous reading in a persistent global (`previousPin8State`) and fire only on the
falling edge — `previousPin8State == HIGH && currentPin8State == LOW`. That toggles
`ledOn` exactly once per press, and you must update the "previous" variable every loop.
Mechanical **bounce** can still sneak in extra edges, which is the cliff-hanger that
motivates debouncing later.
