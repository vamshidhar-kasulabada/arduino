# Hardware Interrupts & ISRs (Challenge #17)

A companion to `17_interrupt_button.ino`. This sketch toggles behaviour on a button
press *without ever checking the button in a loop*. That magic is a **hardware
interrupt**. This note explains interrupts, ISRs, `attachInterrupt`, and the `volatile`
keyword from the ground up, tied to the code you wrote.

---

## 1. Polling vs. interrupts

**Polling** means repeatedly asking "is the button pressed yet?" in `loop()`:

```cpp
// the OLD way (not used here)
void loop() {
  if (digitalRead(buttonPin) == LOW) { ... }   // ask, ask, ask, ask...
}
```

The CPU wastes cycles asking, and if it's busy doing something slow it can *miss* a quick
press. An **interrupt** flips this around: the hardware watches the pin for you and
*taps the CPU on the shoulder* the instant the pin changes. The CPU only does work when
something actually happened — this is **event-driven programming**.

```
POLLING                          INTERRUPT
CPU: "pressed?" no               CPU: ...doing other work...
CPU: "pressed?" no                        |
CPU: "pressed?" no               pin changes --> HARDWARE interrupts CPU
CPU: "pressed?" YES                       |
                                 CPU jumps to ISR, then resumes
```

---

## 2. What an ISR is

An **ISR** (Interrupt Service Routine) is the function the CPU jumps to when the
interrupt fires. In this sketch it's `buttonPressed`:

```cpp
void buttonPressed() { buttonWasPressed = true; }
```

When the button is pressed, the CPU *drops whatever it was doing*, runs this function,
then returns exactly where it left off. You never call `buttonPressed()` yourself — the
hardware does.

---

## 3. `attachInterrupt`: registering the ISR

This is the line that connects pin 2 to the ISR:

```cpp
attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);
//              \________ which pin ________/      \__ ISR __/   \_ when _/
```

Three parts:

- **`digitalPinToInterrupt(buttonPin)`** — converts pin number 2 into the chip's
  *interrupt number*. (On an Uno only pins 2 and 3 support external interrupts; always
  use this macro instead of hard-coding a number.)
- **`buttonPressed`** — the ISR, passed as a *function pointer* (note: no `()` — you're
  handing over the function, not calling it).
- **`FALLING`** — *when* to fire. With `INPUT_PULLUP` the pin idles HIGH and goes LOW
  when pressed, so the **falling** edge (HIGH -> LOW) is the moment of the press:

```cpp
pinMode(buttonPin, INPUT_PULLUP);   // idle HIGH; pressed = LOW
```

```
Pin voltage:  HIGH ----+        +----  (released)
                       |        |
                       +--------+      (pressed = LOW)
                       ^
                  FALLING edge --> fires buttonPressed()
```

`INPUT_PULLUP` uses the chip's *internal* resistor to hold the pin HIGH, so you don't
need an external resistor and the pin never floats to a random value.

---

## 4. The `volatile` keyword

Look closely at the flag the ISR sets:

```cpp
volatile bool buttonWasPressed = false;
```

Why `volatile`? The compiler optimizes aggressively. It sees `loop()` reading
`buttonWasPressed` and, not realizing the ISR can change it "out of nowhere," might cache
the value in a register and never re-read memory — so your `if` would *never* see the
press. `volatile` tells the compiler: **"this variable can change behind your back; read
it from memory every single time."** Any variable shared between an ISR and the main code
*must* be `volatile`.

```
   loop() reads it  <-----  buttonWasPressed (in RAM)  <-----  ISR writes it
                  the compiler must NOT cache this — hence volatile
```

---

## 5. The flag pattern: keep ISRs short

The ISR does the absolute minimum — it sets one flag and returns:

```cpp
void buttonPressed() { buttonWasPressed = true; }   // that's ALL it does
```

The *real* work happens later, back in `loop()`:

```cpp
void loop() {
  if (buttonWasPressed) {
    Serial.println("Pressed!");
    buttonWasPressed = false;   // consume the flag
  }
}
```

This **set-a-flag-in-the-ISR, handle-it-in-loop** split is the most important embedded
pattern in this lesson. Why keep the ISR tiny?

- While an ISR runs, **other interrupts are blocked** — including the timer that drives
  `millis()`. A slow ISR makes the whole system lag or lose time.
- The ISR interrupts your code at an *unpredictable* point, so it must not leave anything
  half-finished.

Hence the rule from the header: **do not use `delay()` inside the interrupt.** `delay()`
itself relies on interrupts to count time, so calling it from an ISR can hang the chip.
`Serial.println` is also too slow/unsafe for an ISR — that's exactly why printing is left
to `loop()`.

---

## 6. ISR signature rules

An ISR registered with `attachInterrupt` must be `void f()` — **no arguments, no return
value**. There's nobody to pass arguments *to* an ISR (the hardware calls it) and nobody
to receive a return value. That's why `buttonPressed` is a bare `void buttonPressed()`,
and why it communicates only through the shared `volatile` flag.

---

## 7. A note on switch bounce

Real buttons "bounce" — one physical press can produce several rapid HIGH/LOW flickers,
firing the ISR multiple times. This sketch doesn't debounce (it just sets a flag, so
extra fires are harmless here), but in projects where each press must count exactly once
you'd debounce — e.g. ignore further edges within a few milliseconds, checking
`millis()` inside the ISR or in `loop()`.

---

## Gotchas

| Pitfall | Why it bites | Fix |
|---|---|---|
| Forgetting `volatile` on the shared flag | Compiler caches it; `loop()` never sees the press | Mark ISR-shared vars `volatile` |
| `delay()` / heavy work in the ISR | Blocks other interrupts, can hang `millis()`/the chip | Set a flag; do work in `loop()` |
| Hard-coding the interrupt number | Pin-to-IRQ mapping isn't 1:1 across boards | Use `digitalPinToInterrupt(pin)` |
| Passing `buttonPressed()` to `attachInterrupt` | That *calls* it; you must pass the pointer | Pass `buttonPressed` (no `()`) |
| Wrong edge mode | `RISING` would fire on release, not press | `INPUT_PULLUP` + `FALLING` = press |
| Not clearing the flag | Handler runs forever | `buttonWasPressed = false;` after handling |

---

## TL;DR

A hardware interrupt lets the chip *react* to a pin change instead of constantly polling
for it. `attachInterrupt(digitalPinToInterrupt(2), buttonPressed, FALLING)` tells the
hardware to run the ISR `buttonPressed` on the press edge. The ISR must be tiny: it just
sets a `volatile bool buttonWasPressed = true` and returns, leaving the slow work
(`Serial.println`) to `loop()`. `volatile` guarantees `loop()` actually re-reads the
flag, and keeping the ISR short keeps the rest of the system (including `millis()`)
running smoothly.
