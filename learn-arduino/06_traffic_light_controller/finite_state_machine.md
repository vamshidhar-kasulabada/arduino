# Finite State Machines with Enums (Challenge #6)

A companion to `06_traffic_light_controller.ino`. A traffic light is the textbook finite
state machine: it cycles through named phases, each with its own duration, and reacts to a
pedestrian button without ever freezing. This note explains the FSM, the `enum`, and the
non-blocking timing that drive it — tied to your code.

---

## 1. The states, named with an `enum`

Challenge #5 used magic numbers (`0`, `1`, `2`). Here we upgrade to an **enum** so each
state has a self-documenting name:

```cpp
enum TrafficLightState { RED, GREEN, YELLOW, RED_YELLOW, PEDESTRIAN_RED };
TrafficLightState state = RED;
```

An `enum` is a list of named integer constants. By default `RED == 0`, `GREEN == 1`, and so
on — but you almost never care about the numbers; you read and write the *names*. The
variable `state` can only hold one of these five values, so the compiler helps you catch
typos and the code reads like the spec.

> C++ angle: this is a plain (unscoped) C-style `enum`, so the names leak into the
> surrounding scope and implicitly convert to `int`. Modern C++ offers `enum class
> TrafficLightState { ... }` for stronger typing (you'd then write `TrafficLightState::RED`),
> which prevents accidental mixing with other integers. For a small sketch the plain enum is
> fine and idiomatic Arduino.

---

## 2. The FSM ingredients

Same three ingredients as any state machine:

1. **States** — the five `enum` values.
2. **Transitions** — the rules that reassign `state`.
3. **Events** — a timer elapsing (`millis() - startTime >= duration`) or the pedestrian
   button being pressed.

The normal cycle plus the pedestrian detour:

```
        5s            2s            5s            2s
  ┌──> RED ──> RED_YELLOW ──> GREEN ──> YELLOW ──┐
  │     │                                         │
  │     │ pedestrianWaiting?                      │
  │     v (yes)                                   │
  │ PEDESTRIAN_RED ──┐                            │
  │   (extra 5s)     │                            │
  │                  v                            │
  └── RED_YELLOW <───┘   (rejoins normal cycle)   │
  ^                                               │
  └───────────────────────────────────────────────┘
```

Exactly one state is active at a time, and `loop()` runs the matching `case` until a
transition fires.

---

## 3. The dispatch: `switch (state)`

Each loop, a `switch` jumps to the code for the current state:

```cpp
switch (state) {
case RED:
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  if (millis() - startTime >= 5000) {
    if (pedestrianWaiting) {
      state = PEDESTRIAN_RED;
    } else {
      state = RED_YELLOW;
    }
    startTime = millis();
  }
  break;
...
}
```

Every `case` has the same shape — **the FSM template**:

1. **Drive the outputs** for this state (which LEDs on/off). This runs every loop, harmlessly
   re-asserting the same pin levels.
2. **Check the transition condition** (timer elapsed).
3. **On transition:** set `state` to the next phase *and* re-stamp `startTime = millis()` so
   the new state's timer starts fresh.

The `break;` is essential — without it, C++ "falls through" into the next `case` and you'd
run several states' code in one loop.

---

## 4. "Each state manages its own duration"

Notice the durations live inside each `case`, matching the spec:

```cpp
case RED:        ... if (millis() - startTime >= 5000) ...   // 5 s
case RED_YELLOW: ... if (millis() - startTime >= 2000) ...   // 2 s
case GREEN:      ... if (millis() - startTime >= 5000) ...   // 5 s
case YELLOW:     ... if (millis() - startTime >= 2000) ...   // 2 s
case PEDESTRIAN_RED: ... if (millis() - startTime >= 5000) ...  // extra 5 s
```

There's **one shared `startTime`** that gets re-stamped on every transition. That's the
non-blocking timing engine: instead of `delay(5000)`, each state records when it began and
checks `millis() - startTime >= duration`. The subtract-first form is overflow-safe across
the 49.7-day `millis()` wrap (see Challenge #4 notes for why).

```
RED begins                         transition
   |---------- 5000 ms ----------|
   startTime                    millis() - startTime >= 5000  -> next state
                                 (and startTime = millis() again)
```

Because nothing blocks, the pedestrian button is still polled every loop, even mid-phase.

---

## 5. The pedestrian feature: deferred, latched events

The spec is subtle: a button press must **not interrupt** the current phase. It's recorded
and *honored later*, only when the light next reaches RED. That's a **latched event**.

Detection happens at the very top of `loop()`, outside the switch, so it runs in every
state:

```cpp
int currentBtnState = digitalRead(2);
if (prevBtnState == HIGH && currentBtnState == LOW) {   // falling edge
  pedestrianWaiting = true;                              // LATCH the request
}
prevBtnState = currentBtnState;
```

`pedestrianWaiting` is a `bool` flag that *remembers* the press happened, surviving across
many loops and state changes. The flag is only *consumed* at the RED transition:

```cpp
case RED:
  ...
  if (millis() - startTime >= 5000) {
    if (pedestrianWaiting) {
      state = PEDESTRIAN_RED;     // detour: keep red longer
    } else {
      state = RED_YELLOW;         // normal
    }
    startTime = millis();
  }
```

And cleared when the crossing finishes, so it doesn't trigger again next cycle:

```cpp
case PEDESTRIAN_RED:
  ...
  if (millis() - startTime >= 5000) {
    pedestrianWaiting = false;    // consume the request
    state = RED_YELLOW;
    startTime = millis();
  }
```

This "set a flag now, act on it at a safe point" pattern is exactly how real firmware
handles asynchronous events — you defer the response to a state where it's safe to act.

### Edge detection + `INPUT_PULLUP` (again)

```cpp
pinMode(2, INPUT_PULLUP);   // idle reads HIGH; button to GND reads LOW when pressed
```

`prevBtnState == HIGH && currentBtnState == LOW` is the falling edge. Without it, holding
the button would re-set `pedestrianWaiting = true` every loop (harmless here since it's just
re-setting `true`, but the edge is still the correct, intentional way to detect a press).

---

## 6. Why `PEDESTRIAN_RED` is a separate state (not just a longer RED)

Both RED and PEDESTRIAN_RED light only the red LED — so why two states? Because **states
encode intent and different exits**:

- `RED` exits after 5 s to RED_YELLOW (or detours to PEDESTRIAN_RED).
- `PEDESTRIAN_RED` exits after its *extra* 5 s straight to RED_YELLOW, and clears the flag.

Splitting them keeps each `case` simple and prevents tangled conditions like "stay red for 5
or 10 seconds depending on a flag, but only clear the flag in the second half." One concern
per state is the whole reason FSMs stay readable.

---

## 7. Embedded relevance

- **`const short` pin numbers.** `const short redPin = 8;` names the wiring once. `short`
  (16-bit) is plenty for a pin number and documents intent; `const` lets the compiler fold it
  into the instruction, costing no RAM.
- **Outputs re-driven every loop.** Calling `digitalWrite` each iteration is cheap and makes
  state self-correcting — even if a glitch flipped a pin, the next loop restores it.
- **No RTOS needed.** A single `enum` + `switch` + one `startTime` cooperatively runs a
  real-time sequence on a single thread, leaving the CPU free to poll the button.
- **Deferred event handling** (the latch) mirrors how ISRs set a flag for the main loop to
  process later — never doing heavy work at an unsafe moment.

---

## Gotchas

| Pitfall | Fix |
|---|---|
| Missing `break;` in a `case` | Falls through into the next state's code — always `break` |
| Forgetting `startTime = millis()` on transition | New state's timer never resets; it expires instantly |
| `millis() >= startTime + 5000` | Overflows near 49.7 days — use `millis() - startTime >= 5000` |
| Acting on the button immediately | Spec says defer — latch `pedestrianWaiting`, consume at RED |
| Never clearing `pedestrianWaiting` | Detour fires every cycle — clear it in PEDESTRIAN_RED |
| Magic numbers for states | Use the `enum` names (`RED`, `GREEN`, ...) |
| Button counted while held | Edge-detect with `prevBtnState`; update it every loop |

---

## TL;DR

A finite state machine is "exactly one named state at a time, plus rules to move between
them." Here an `enum TrafficLightState` names five phases, a `switch (state)` drives the
right LEDs and checks that phase's own timer with the overflow-safe `millis() - startTime >=
duration` idiom, and transitions re-stamp `startTime`. The pedestrian button is a **latched,
deferred event**: a falling edge sets `pedestrianWaiting`, which is honored only when the
light next hits RED (routing to `PEDESTRIAN_RED`) and then cleared. Non-blocking throughout,
so the button is always responsive.
