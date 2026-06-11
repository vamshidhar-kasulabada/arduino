# The Controller Pattern (Challenge #16)

A companion to `16_traffic_light_controller_class.ino`. In earlier lessons the traffic
light's state machine lived loose in `loop()`. Here it moves *inside a class* —
`TrafficLightController`. This note explains why that matters, from the ground up, then
ties each idea back to the code you wrote.

---

## 1. The core idea

A **controller** is a class that owns an entire piece of behaviour — the hardware it
drives, the *state* it's in, and the *timing* that moves it between states — and exposes
one tiny public verb. For this sketch that verb is `update()`:

```cpp
void loop() { controller.update(); }
```

That's the whole `loop()`. Everything else is hidden. The controller is a self-contained
unit you can drop into any sketch and call once per loop.

---

## 2. Composition: classes owning classes

Notice that `TrafficLightController` does **not** talk to pins directly. It *has-a*
`TrafficLight`, which in turn *has-a* three `Led`s:

```cpp
class TrafficLightController {
private:
  TrafficLight trafficLight;   // controller owns a TrafficLight
  ...
};
```

This is **composition** ("has-a"), the opposite of inheritance ("is-a"). Each layer adds
meaning and hides the layer below:

```
TrafficLightController   "what the signal is doing"  (state + timing)
        owns
   TrafficLight          "which lamps are lit"        (showRed, showGreen...)
        owns
   Led redLed            "one pin, on or off"          (on, off, toggle)
   Led yellowLed
   Led greenLed
```

The controller never calls `digitalWrite` — it says `trafficLight.showRed()`, and
`TrafficLight` says `redLed.on()`, and `Led` finally does `digitalWrite(pin, state)`.
Each class only knows about the one directly below it.

---

## 3. Private state = encapsulation

The challenge rules forbid global `state` and `startTime` variables. Instead they become
**private members**:

```cpp
private:
  unsigned long time;   // WHEN the current state started
  State state;          // WHICH state we're in
```

Because they're `private`, nothing outside the class can poke them. The only way to drive
the machine is the public API (`update()`), and the only way state changes is through the
controller's own logic. That is **encapsulation**: the data and the code that's allowed to
touch it are sealed together. Compare the two worlds:

```
GLOBAL STATE (old way)            OWNED STATE (this lesson)
+-----------------------+         +-----------------------------+
| unsigned long time;   |         | TrafficLightController      |
| int state;            |         |  - time   (private)         |
| ...in loop()...       |         |  - state  (private)         |
| anyone can corrupt it |         |  + update() (only door in)  |
+-----------------------+         +-----------------------------+
```

Want a *second* signal at a different intersection? With globals you'd duplicate every
variable. With a controller you just write `TrafficLightController controller2(5, 6, 7);`.

---

## 4. The finite state machine (FSM)

A **finite state machine** is a system that is always in exactly one of a fixed set of
states, and moves between them on defined conditions. The states are an `enum`:

```cpp
enum State { RED, RED_YELLOW, GREEN, YELLOW };
```

The transitions form a cycle:

```
   +--> RED (5s) --> RED_YELLOW (2s) --> GREEN (5s) --> YELLOW (2s) --+
   |                                                                  |
   +------------------------------------------------------------------+
```

`update()` is the FSM engine. Each `case` does two jobs: **drive the output** for the
current state, and **check whether it's time to move on**:

```cpp
case RED:
  trafficLight.showRed();              // 1. output for this state
  if (millis() - time >= 5000) {       // 2. has 5s elapsed?
    transitionTo(RED_YELLOW);          //    move to next state
  }
  break;
```

---

## 5. The transition helper

Every transition has to do the *same* two things: change the state, and reset the clock.
Forgetting the clock reset is a classic FSM bug, so the sketch factors it into one place:

```cpp
void transitionTo(State nextState) {
  state = nextState;
  time = millis();      // restart the timer for the NEW state
}
```

Now each `case` just says `transitionTo(GREEN)` and can't forget to reset `time`. This is
the same DRY instinct that pushed pins into `Led` — repeated logic gets a name.

---

## 6. Non-blocking timing (no `delay()`)

The rules ban `delay()`. A `delay(5000)` would freeze the *entire* chip for 5 seconds —
no other task could run, no button could be read. Instead the controller compares
timestamps:

```cpp
if (millis() - time >= 5000) { ... }
```

`millis()` returns milliseconds since boot. `time` is *when this state began*. The
subtraction gives elapsed time. Because `update()` returns immediately when the interval
hasn't passed, `loop()` keeps spinning freely — the chip stays responsive. (And
`millis() - time` is robust even when `millis()` overflows back to 0 after ~49 days,
because unsigned subtraction wraps correctly.)

---

## 7. Construction: initializer lists

The constructor wires everything up before the body even runs:

```cpp
TrafficLightController(uint8_t redPin, uint8_t yelloPin, uint8_t greenPin)
    : trafficLight(redPin, yelloPin, greenPin), time(millis()), state(RED) {}
```

Everything after the `:` is the **member initializer list**. It's the only way to
construct the owned `trafficLight` (a class member with no default constructor must be
built here), and it sets the machine's starting condition: clock = now, state = `RED`.
The body `{}` is empty because there's nothing left to do.

---

## 8. Why this is good firmware architecture

This pattern scales. Real firmware is a handful of controllers, each owning its own
state, all pumped from a non-blocking `loop()`:

```cpp
void loop() {
  trafficController.update();
  motorController.update();
  displayController.update();
}
```

No globals, no `delay()`, each module testable and reusable in isolation. The traffic
light is the toy version of how production embedded systems are structured.

---

## Gotchas

| Pitfall | Why it bites | Fix |
|---|---|---|
| Using `delay()` inside a state | Freezes the whole chip; nothing else can run | Compare `millis() - time` |
| Forgetting `time = millis()` on transition | New state inherits old timestamp; fires instantly | Always go through `transitionTo()` |
| Making `state`/`time` public/global | Anyone can corrupt the FSM | Keep them `private` members |
| `==` vs `>=` on the timer | A missed loop tick can skip an exact `==` match | Use `>=` for the elapsed check |
| Missing `break;` in `switch` | Falls through into the next state's code | One `break;` per `case` |

---

## TL;DR

A **controller** packages a whole state machine — the hardware it drives, the state it's
in, and the timing that advances it — behind one public method. `TrafficLightController`
*owns* a `TrafficLight` (composition), keeps `state` and `time` *private*
(encapsulation), and advances itself with non-blocking `millis()` checks inside
`update()`. The reward: `loop()` shrinks to `controller.update();`, and the module is
reusable, testable, and impossible to corrupt from outside.
