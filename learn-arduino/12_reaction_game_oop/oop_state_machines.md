# OOP + State Machines (Challenge #12)

A companion to `12_reaction_game_oop.ino`. This challenge rebuilds the reaction-timer
game by **composing reusable classes** (`Led`, `Button`) with a **finite state machine**.
This note shows how those two ideas fit together: the classes handle *hardware*, the
state machine handles *logic*, and neither leaks into the other.

---

## 1. The big idea: separate "how" from "what"

The earlier reaction game mixed everything together — `digitalRead`, debounce timers,
and `digitalWrite` all tangled with the game rules. Here those concerns are split:

- **`Led` and `Button`** (in `Led.h` / `Button.h`) answer *how* to talk to hardware.
- **The state machine** in `loop()` answers *what* the game should do next.

The sketch's own rules spell this out: never call `digitalRead()` or `digitalWrite()`
directly — interact only through `button.wasPressed()`, `led.on()`, `led.off()`. The
game code is left to focus on *states, timers, and reaction measurement*.

---

## 2. Composition via header files

The two classes live in their own headers and are pulled in with:

```cpp
#include "Button.h"
#include "Led.h"
```

Both headers start with `#pragma once` — a guard that stops the file being included
twice in one translation unit (which would cause "class redefined" errors). Quotes (`"..."`)
mean "look in my project folder first," versus angle brackets (`<Arduino.h>`) for
system/library headers.

The game then **composes** the objects it needs as globals:

```cpp
Led led(8);
Button button(2);
```

This is **composition**: the game is *built out of* a `Led` and a `Button`. It reuses the
exact same debouncing `Button` from Challenge #10 and `Led` from #11 — unchanged. Good
abstractions mean an old app gets rebuilt with cleaner code and zero copy-paste.

---

## 3. What a state machine is

A **finite state machine (FSM)** is a system that is always in exactly *one* of a fixed
set of states, and moves between them on **events**. The states are named by an `enum`:

```cpp
enum GameState {
  WAITING_FOR_BTN_PRESS,
  WAITING_FOR_LED,
  WAITING_FOR_REACTION,
};

GameState gameState = WAITING_FOR_BTN_PRESS;
```

The single variable `gameState` *is* the machine's memory — it remembers where we are
between iterations of `loop()`. Using an `enum` (rather than `int 0/1/2`) makes the code
self-documenting and lets the compiler warn on a missing `switch` case.

---

## 4. The state diagram

```
                press button
   ┌─────────────────────────────────────────────┐
   │                                              │
   ▼                                              │
┌──────────────────────┐  press   ┌──────────────────┐
│ WAITING_FOR_BTN_PRESS │ ───────► │  WAITING_FOR_LED  │
└──────────────────────┘          └──────────────────┘
   ▲      ▲                          │            │
   │      │  "Too Early!"            │ random     │ press
   │      └──────(press)─────────────┘ 2-8 s      │ before LED?
   │                                   elapsed    │ (back to start)
   │                                   ▼
   │   print reaction time   ┌──────────────────────┐
   └─────────────(press)──── │  WAITING_FOR_REACTION │  (LED is ON)
                             └──────────────────────┘
```

Each box is a state; each arrow is a **transition** triggered by an event (a button
press, or a timer expiring).

---

## 5. The `switch` that implements it

`loop()` runs continuously and, each pass, executes *only the branch for the current
state*:

```cpp
void loop() {
  switch (gameState) {
  case WAITING_FOR_BTN_PRESS:
    if (button.wasPressed()) {
      gameState = WAITING_FOR_LED;
      btnPressedAt = millis();
      value = random(2000, 8000);   // random 2-8 s wait
    }
    break;

  case WAITING_FOR_LED:
    if (millis() - btnPressedAt >= value) {  // wait elapsed -> light it
      ledOnAt = millis();
      led.on();
      gameState = WAITING_FOR_REACTION;
    } else if (button.wasPressed()) {         // pressed too soon
      Serial.println("Too Early!!");
      gameState = WAITING_FOR_BTN_PRESS;
    }
    break;

  case WAITING_FOR_REACTION:
    if (button.wasPressed()) {
      btnPressedAt = millis();
      reactionTime = btnPressedAt - ledOnAt;  // measure!
      Serial.print(reactionTime);
      Serial.println("ms");
      gameState = WAITING_FOR_BTN_PRESS;
      led.off();
    }
    break;
  }
}
```

A transition is just **`gameState = <new state>;`**. Notice each state reacts to
*different* events: in `WAITING_FOR_LED` a press is a foul ("Too Early!"), but in
`WAITING_FOR_REACTION` the very same press is the winning input being measured. The
state gives the press its meaning.

---

## 6. Timing & measurement with `millis()`

Three `unsigned long` timestamps cooperate, and no `delay()` is ever used:

| Variable | Set when | Used for |
|---|---|---|
| `btnPressedAt` | the start press / reaction press | start of the random wait; later the reaction press time |
| `value` | start press | the random target wait, `random(2000, 8000)` ms |
| `ledOnAt` | the moment the LED lights | the zero point for reaction timing |

The wait is non-blocking: `millis() - btnPressedAt >= value`. The reaction time is a
simple difference: `reactionTime = btnPressedAt - ledOnAt`. Because `delay()` is never
called, the machine can simultaneously watch for the "Too Early!" press *while* counting
down the random wait — impossible with blocking delays.

`randomSeed(analogRead(A0))` in `setup()` reads a floating (unconnected) analog pin for
electrical noise, so the 2–8 s wait differs every power-up. Without it, `random()`
produces the *same* sequence every boot and the game becomes predictable.

```
  start press        LED on            reaction press
      │                 │                    │
      ▼                 ▼                    ▼
  btnPressedAt ──── value ms ──── ledOnAt ── reactionTime ──►
                  (random 2-8 s)            (what we report)
```

---

## 7. Why OOP + FSM is the winning combination

Each does one job, and they stay out of each other's way:

- The **FSM** never worries about switch bounce — `button.wasPressed()` already returns a
  clean, single event per press (its 20 ms debounce is sealed inside the class).
- The **classes** never know a game exists — `Led` just turns a pin on/off; it could
  drive a traffic light tomorrow.

```
   ┌─────────────────── loop() : the GAME LOGIC ───────────────────┐
   │  gameState  +  millis() timers  +  reaction math              │
   │                                                               │
   │     ▲ asks "did a press happen?"      ▼ commands "light up"   │
   └─────│────────────────────────────────│──────────────────────┘
         │                                │
   ┌─────┴──────┐                   ┌─────┴──────┐
   │  Button     │  (debounce here) │   Led       │  (pin I/O here)
   └────────────┘                   └────────────┘
```

This separation is the entire point of the challenge: when hardware is abstracted, the
application is *just* states and timers — readable, testable, and easy to extend.

---

## Gotchas

| Pitfall | What happens | Fix |
|---|---|---|
| Forgetting `break;` in a `case` | Falls through into the next state's code | End every case with `break;` |
| Forgetting `gameState = ...` | Stuck in one state forever | Every event handler must transition |
| Using `delay()` for the random wait | Can't catch the "Too Early!" press | Use `millis() - btnPressedAt >= value` |
| Skipping `randomSeed()` | Same wait sequence every boot | Seed from `analogRead(A0)` noise |
| Comparing timestamps with `<` | Breaks on `millis()` rollover (~49 days) | Subtract: `millis() - start >= span` |
| Putting debounce/`digitalRead` in `loop()` | Re-tangles hardware with logic | Keep it inside `Button` / `Led` |
| Storing `value`/times as `int` | Overflow / negative math on long waits | Use `unsigned long` for `millis()` values |

---

## TL;DR

The reaction game is **two reusable classes** (`Led`, `Button`, pulled in via `#include`
and composed as globals) driving a **three-state finite state machine** (`enum GameState`
in a `switch`). The classes own *how* to touch hardware — debounced presses, clean LED
control — while the FSM owns *what* happens next, using `millis()` timestamps and
`random()` for the wait. The same button press means "start," "too early," or "measure me"
depending on the current state. Clean abstractions turn the whole game into nothing but
states, timers, and a subtraction.
