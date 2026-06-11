# Composition & Member-Initializer Lists (Challenge #13)

A companion to `13_traffic_light_class.ino`. The `TrafficLight` class in this challenge
works because of **composition** — it is built *out of* three `Led` objects. This note
explains composition and the **member-initializer list** from the ground up, then ties
each idea back to the code you wrote.

---

## 1. The core idea: "has-a"

A traffic light *is not* an LED. A traffic light **has** three LEDs. That "has-a"
relationship is **composition**: a class owns other class objects as its members.

```cpp
class TrafficLight {
private:
  Led redLed;      // TrafficLight HAS-A red Led
  Led yellowLed;   // HAS-A yellow Led
  Led greenLed;    // HAS-A green Led
};
```

You are stacking objects like Lego. The small object (`Led`) already knows how to talk
to one pin. The big object (`TrafficLight`) just arranges three of them into a higher-level
behavior. You never touch `digitalWrite()` again — that detail lives *inside* `Led`.

```
        TrafficLight (the "has-a" owner)
        +-------------------------------+
        |  Led redLed     -> pin 8      |
        |  Led yellowLed  -> pin 9      |
        |  Led greenLed   -> pin 10     |
        +-------------------------------+
            ^ owns 3 Led objects by value
```

---

## 2. Composition vs. inheritance

There are two ways to reuse a class:

- **Inheritance** ("is-a"): `class BlinkingLed : public Led`. A `BlinkingLed` *is an* `Led`.
- **Composition** ("has-a"): `TrafficLight` *has* three `Led`s.

This challenge uses composition, and that is usually the right default. A `TrafficLight`
is clearly not a kind of `Led`, so inheritance would be wrong. The guideline "**prefer
composition over inheritance**" exists because composition is more flexible and keeps
each class small and focused.

---

## 3. The problem: members need constructor arguments

Your `Led` class has **no default constructor**. The only constructor it offers is:

```cpp
Led(uint8_t p) { pin = p; state = false; pinMode(pin, OUTPUT); write(); }
```

That means you cannot just declare `Led redLed;` — there is no way to build an `Led`
without telling it which pin. So when `TrafficLight` owns three `Led`s, *something* has
to supply each pin number **before** the body of the `TrafficLight` constructor even runs.

C++ builds member objects in declaration order, *as part of constructing the outer
object*. By the time you reach the `{ }` body, every member must already be fully built.
That is exactly what the member-initializer list is for.

---

## 4. The member-initializer list

This is the key line from your sketch:

```cpp
TrafficLight(uint8_t redPin, uint8_t yelloPin, uint8_t greenPin)
    : redLed(redPin), yellowLed(yelloPin), greenLed(greenPin) {}
//  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  the initializer list: forwards each pin into each Led's constructor
```

Read it as: "before my body runs, build `redLed` by calling `Led(redPin)`, build
`yellowLed` by calling `Led(yelloPin)`, build `greenLed` by calling `Led(greenPin)`."

```cpp
TrafficLight(...) : redLed(redPin), yellowLed(yelloPin), greenLed(greenPin) { }
//                  ^member^ ^arg^                                          ^empty body
```

The body `{}` is empty here because there is nothing left to do — each `Led` constructor
already called `pinMode(... OUTPUT)` and wrote the pin LOW for us.

### Why not just assign in the body?

You might be tempted to write:

```cpp
TrafficLight(uint8_t redPin, ...) {
  redLed = Led(redPin);   // ERROR / wasteful
}
```

This does **not** work cleanly: `redLed` would first have to be *default-constructed*
(impossible — `Led` has no default constructor), then *reassigned*. The initializer list
constructs each member **once, directly with the right value**. Assignment in the body is
"build a default, then overwrite it" — two steps instead of one.

> **Rule:** members that are `const`, are references, or lack a default constructor
> **must** be set in the initializer list. `Led` falls into that last category.

---

## 5. Initialization order gotcha

Members are initialized in the order they are **declared in the class**, *not* the order
they appear in the initializer list. In your code the declaration order is:

```cpp
Led redLed;       // built 1st
Led yellowLed;    // built 2nd
Led greenLed;     // built 3rd
```

So even if you wrote `: greenLed(greenPin), redLed(redPin), ...`, `redLed` would still be
constructed first. It does not matter here because the three `Led`s are independent, but
if one member depended on another, relying on list order would be a subtle bug. Keep the
list in declaration order to avoid confusion (and the compiler warning).

---

## 6. Encapsulation: the payoff

Because the `Led`s are `private`, the outside world cannot poke them directly. It can only
go through the public methods you expose:

```cpp
void showRed()       { redLed.on();  yellowLed.off(); greenLed.off(); }
void showYellowRed() { redLed.on();  yellowLed.on();  greenLed.off(); }
void showGreen()     { redLed.off(); yellowLed.off(); greenLed.on();  }
```

Notice each method enforces a **valid combination** — exactly one (or two) lights at a
time. A caller can never accidentally turn on red and green together, because they never
touch the LEDs; they call `trafficLight.showRed()`. That is encapsulation: the class
guards its own invariants.

---

## 7. How this drives your state machine

In `loop()`, the high-level object hides all the pin work. The state machine just picks
*which named state* to show and uses `millis()` for timing:

```cpp
case RED:
  trafficLight.showRed();
  if (millis() - time >= 5000) { state = YELLOW_RED; time = millis(); }
  break;
```

The loop reads almost like the spec ("RED for 5 s, then RED+YELLOW for 2 s..."). All the
`digitalWrite` / `pinMode` noise is buried two layers down (`TrafficLight` -> `Led`). That
layering is the whole point of composition.

---

## 8. Embedded relevance

- **Hardware abstraction layers (HAL).** Real firmware composes drivers exactly this way:
  a `Motor` *has-a* `PwmChannel` and a `GpioPin`; a `Robot` *has-a* `Motor[]`. Each layer
  hides the registers below it.
- **Reusable components.** The same `Led.h` is dropped unchanged into challenges 13, 14,
  and 15. Composition lets one well-tested building block serve many sketches.
- **Cost on AVR.** `TrafficLight` stores three `Led`s **by value** (not pointers), so it is
  just three `Led`s laid out back-to-back in memory — no heap, no `new`, fully
  deterministic. Each `Led` is a `uint8_t pin` + a `bool state`, so the whole
  `TrafficLight` is only a handful of bytes.

---

## 9. Gotchas cheat-sheet

| Symptom | Cause | Fix |
|---|---|---|
| `no matching function for call to 'Led::Led()'` | Member has no default constructor and you forgot the initializer list | Initialize it: `: redLed(pin)` |
| Compiler warning "will be initialized after" | Initializer list order differs from declaration order | Reorder the list to match declaration order |
| LED setup seems to "not take" | Assigned in `{}` body instead of the list | Use the initializer list so the `Led` ctor runs once with the real pin |
| Can't poke `redLed` from `loop()` | It's `private` (good!) | Call a public method like `showRed()` |

- The list runs **before** the constructor body, in **declaration order**.
- Members owned **by value** are constructed and destroyed automatically with the owner.

---

## TL;DR

Composition means a class **owns other objects** ("has-a"). `TrafficLight` owns three
`Led`s and exposes safe, named methods (`showRed()`, ...) instead of raw pins. Because
`Led` needs a pin at construction time, you must hand those pins over in the
**member-initializer list** (`: redLed(redPin), ...`), which builds each member exactly
once, in declaration order, before the constructor body runs. The result is a clean,
reusable, layered design where the state machine reads like the spec and never sees a
single `digitalWrite`.
