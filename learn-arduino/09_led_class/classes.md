# Classes, Constructors & Encapsulation (Challenge #9)

A companion to `09_led_class.ino`. This challenge turns a raw pin into a self-contained
`Led` object. This note explains classes from the ground up, then ties each idea back to
the code you wrote.

---

## 1. From loose variables to an object

Before this lesson, controlling an LED meant juggling separate facts: which pin, whether
it's on, and remembering to call `pinMode` and `digitalWrite` correctly every time. A
**class** bundles that *data* together with the *functions* that operate on it into one
type:

```cpp
class Led {
private:
  uint8_t pin;     // which pin this LED is wired to
  bool state;      // is it currently on?
  void write() { digitalWrite(pin, state); }
public:
  Led(uint8_t p);  // constructor
  void toggle();
  void on();
  void off();
  bool isOn();
};
```

A class is the **blueprint**; an **object** is a thing built from it. `redLed`,
`yellowLed`, and `greenLed` are three independent objects, each carrying its own `pin`
and `state`.

```
        Led (blueprint)
            │  build instances
   ┌────────┼─────────┐
 redLed   yellowLed  greenLed
 pin=8     pin=9      pin=10
 state=?   state=?    state=?     each object has its OWN copy of the data
```

---

## 2. The constructor: setup that can't be forgotten

A **constructor** is a special function with the same name as the class and no return
type. It runs **automatically** the moment an object is created, so an `Led` can never
exist in an unconfigured state:

```cpp
Led(uint8_t p) {
  pin = p;             // remember which pin
  state = false;       // start OFF
  pinMode(pin, OUTPUT);// configure the hardware
  write();             // push that OFF state to the pin
}
```

The argument `p` is the pin number you pass in. Creating the objects runs this code three
times, once per LED:

```cpp
Led redLed(8);          // calls Led(8)  -> pin=8, OFF, pinMode done
Led yellowLed = Led(9); // identical: also calls the constructor
Led greenLed = Led(10);
```

The comment in the sketch — `//same as Led redLed = Led(8);` — points out that both forms
do the exact same thing; the first is just shorthand. Notice `pinMode()` is now inside the
constructor, **not** in `setup()`. The object configures its own hardware.

---

## 3. Encapsulation: `private` vs `public`

The class is split into two halves:

- **`private`** members (`pin`, `state`, `write()`) are reachable **only from inside the
  class**. Outside code literally cannot touch them.
- **`public`** members (`Led()`, `toggle()`, `on()`, `off()`, `isOn()`) are the
  **interface** — the buttons the outside world is allowed to press.

```
        ┌──────────── Led ────────────┐
 outside│  public:   on() off()        │
 code   │            toggle() isOn()   │ <- you may call these
 ───────┼─────────────────────────────┤
        │  private:  pin  state        │ <- hidden; only the class touches these
        │            write()           │
        └─────────────────────────────┘
```

This is **encapsulation**: hiding internal data and exposing only safe operations.
`state` can never drift out of sync with the pin, because the *only* ways to change it
(`on`/`off`/`toggle`) all call the private `write()` afterward. The challenge's rule —
"Do not call `digitalWrite()` outside the Led class" — is encapsulation stated as a law:
all hardware access funnels through one place.

---

## 4. Methods: behavior tied to the data

The public methods are tiny but each keeps `state` and the physical pin in lockstep:

```cpp
void toggle() { state = !state; write(); }   // flip and push
void on()     { state = true;   write(); }   // force on
void off()    { state = false;  write(); }   // force off
bool isOn()   { return state;            }   // read-only query
```

When you call `redLed.toggle()`, the method runs *on that specific object* — it reads and
writes `redLed`'s own `pin` and `state`, not yellow's or green's. That's why three objects
can share one class yet behave independently.

`write()` being **private** is deliberate: it's an implementation detail
(`digitalWrite(pin, state)`). Callers shouldn't poke the pin directly; they call `on()`
and trust the class to do the right `write()`. `isOn()` is a **getter** — it exposes the
hidden `state` for reading without letting anyone set it.

---

## 5. Hardware abstraction

The big embedded payoff is **abstraction**. `loop()` reads at a higher level — it talks
about LEDs, not pins or voltages:

```cpp
void loop() {
  if (millis() - time >= 1000) {
    time = millis();
    redLed.toggle();
    yellowLed.toggle();
    greenLed.toggle();
  }
}
```

There is no `pin 8`, no `HIGH`/`LOW`, no `digitalWrite` in sight — those details are
sealed inside `Led`. If the wiring changed (active-low LEDs, a shift register, PWM
dimming), you'd edit `write()` once and every call site keeps working unchanged. That
separation between *what* (toggle the red LED) and *how* (set pin 8 high) is the heart of
driver design.

---

## 6. Non-blocking timing (unchanged from earlier lessons)

The blink cadence still uses `millis()`, never `delay()`:

```cpp
if (millis() - time >= 1000) { time = millis(); /* toggle all three */ }
```

`time` holds the timestamp of the last toggle; the unsigned subtraction is rollover-safe.
Wrapping the LEDs in a class didn't change the timing pattern — it just made the bodies
read as `redLed.toggle()` instead of pin-level fiddling.

---

## 7. Embedded relevance (AVR specifics)

- **Cost.** Each `Led` object is tiny: `uint8_t pin` (1 byte) + `bool state` (1 byte) ≈ 2
  bytes of RAM per LED. The methods compile to essentially the same instructions as the
  hand-written `digitalWrite` — the class is "free" at runtime here.
- **No virtuals = no overhead.** This class has no `virtual` functions, so there's no
  vtable and no indirection. On a 2 KB-RAM Uno that matters.
- **Constructors run before `setup()` finishes its job for you.** Globals like `redLed`
  are constructed during C++ startup, *before* `loop()` runs — which is exactly why
  putting `pinMode` in the constructor works without an explicit call in `setup()`.

---

## Gotchas

| Pitfall | What goes wrong |
|---|---|
| forgetting `public:` | members default to `private` in a `class`; the outside can't call `on()` etc. |
| calling `write()` from outside | it's `private` — compile error (and the point of encapsulation) |
| `Led redLed;` with no argument | no default constructor exists; must pass a pin: `Led redLed(8)` |
| reading `state` instead of `isOn()` | `state` is private; use the getter |
| putting `pinMode` in `setup()` too | redundant — the constructor already did it |
| expecting objects to share `state` | each object has its own copy; `redLed.toggle()` never affects `greenLed` |

---

## TL;DR

A `class` bundles data (`pin`, `state`) with behavior (`on`, `off`, `toggle`). The
**constructor** `Led(uint8_t p)` runs automatically and configures the pin so an `Led` is
always ready to use. **Encapsulation** hides `pin`, `state`, and `write()` behind a small
public interface, enforcing the rule that all `digitalWrite` calls live inside the class.
The result is **hardware abstraction**: `loop()` says `redLed.toggle()` and never thinks
about pins again.
