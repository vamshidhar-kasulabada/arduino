# Digital Output (Challenge #1)

A companion to `01_blink_led.ino`. This sketch is the "hello world" of embedded —
five lines that get a pin to flip a voltage on and off. This note explains what each
line actually does, from the C++ structure down to the wire on pin 13.

---

## 1. The two functions every sketch needs

There is no `main()` in your code. Arduino provides a hidden one that calls two
functions you write:

```cpp
void setup() { ... }   // runs ONCE at power-up / reset
void loop()  { ... }   // runs OVER AND OVER, forever
```

Conceptually the framework's `main()` looks like:

```cpp
int main() {
  init();          // configure timers, ADC, etc.
  setup();         // YOUR one-time config
  for (;;) {       // infinite loop
    loop();        // YOUR repeating code
  }
}
```

So `setup()` is for things you do once (configuring pins), and `loop()` is for the
behaviour that repeats (the blink). On an Arduino Uno there is no operating system —
when `loop()` returns, it is simply called again. There is no "exit".

---

## 2. `pinMode(13, OUTPUT)` — pick a direction

Every general-purpose pin can be either an **input** (it listens to a voltage) or an
**output** (it drives a voltage). You must declare which before you use it:

```cpp
void setup() {
  pinMode(13, OUTPUT);   // pin 13 will DRIVE a voltage
}
```

- `13` is the Arduino pin number. On the Uno, pin 13 is special: it is wired to the
  built-in LED on the board (labelled "L"), so this sketch blinks even with nothing
  plugged in.
- `OUTPUT` is just a named constant (it equals `1`). Behind the scenes this sets a bit
  in the AVR's **Data Direction Register** (`DDRB` for this pin), telling the silicon
  to connect that pin to a driver that can source/sink current.

```
   AVR pin internals (simplified)
   ┌──────────────┐
   │   DDRB bit   │  0 = INPUT  (high-impedance, listens)
   │   for PB5    │  1 = OUTPUT (low-impedance, drives)
   └──────────────┘
            │
        pin 13 ──► LED ──► (resistor) ──► GND
```

---

## 3. `digitalWrite(13, HIGH/LOW)` — set the voltage

Once a pin is an OUTPUT, `digitalWrite` chooses which of two voltages it puts out:

```cpp
digitalWrite(13, HIGH);   // ~5 V  -> LED ON
digitalWrite(13, LOW);    //  0 V  -> LED OFF
```

`HIGH` and `LOW` are constants for `1` and `0`. "Digital" means there are only these
two levels — never anything in between. The LED lights when current flows from the
5 V pin, through the LED, to ground (GND).

| `digitalWrite` | Pin voltage | Built-in LED |
|---|---|---|
| `HIGH` | ~5 V | ON |
| `LOW`  | 0 V  | OFF |

> Hardware note: never wire a raw LED from a pin straight to GND with no resistor on
> an external circuit — the LED will draw too much current. The Uno's *built-in* LED
> already has a series resistor on the board, which is why this bare sketch is safe.

---

## 4. `delay(1000)` — wait, the blunt way

```cpp
digitalWrite(13, HIGH);
delay(1000);              // do nothing for 1000 ms
digitalWrite(13, LOW);
delay(1000);
```

`delay(ms)` blocks: the CPU sits in a tight counting loop for that many milliseconds
and does **nothing else**. `1000` ms = 1 second, matching the challenge's "ON for 1
second, OFF for 1 second".

Here is the resulting waveform on pin 13 over time:

```
 5V ┌──────────┐          ┌──────────┐
    │   ON     │          │   ON     │
 0V ┘          └──────────┘          └────  ...
    |←  1 s   →|←  1 s   →|←  1 s   →|
    HIGH        LOW        HIGH
```

The downside of `delay()` is that while waiting you cannot read a button, talk to
Serial, or blink a *second* LED at a different rate. That limitation is exactly what
later challenges replace with `millis()`-based timing. For a single blinking LED,
though, `delay()` is perfectly fine and easy to read.

---

## 5. Why the loop blinks forever

Put the four statements together and trace the flow:

```cpp
void loop() {
  digitalWrite(13, HIGH);  // LED on
  delay(1000);             // hold 1 s
  digitalWrite(13, LOW);   // LED off
  delay(1000);             // hold 1 s
}                          // loop() returns -> framework calls it again
```

`loop()` finishing is not the end — the hidden `main()` immediately calls it again, so
the on/off/on/off pattern repeats indefinitely. No `while(true)` of your own is needed.

---

## 6. The C++ angle

- These are ordinary C++ functions returning `void`. Arduino sketches are C++ compiled
  by `avr-g++`; the `.ino` is just C++ with the two functions auto-declared for you.
- `#include <Arduino.h>` pulls in the declarations for `pinMode`, `digitalWrite`,
  `delay`, and the `HIGH`/`OUTPUT` constants. The IDE adds this automatically, but it
  is written explicitly here so the file also compiles as plain C++.
- `HIGH`, `LOW`, `OUTPUT` are `#define`/`const` integer constants, not magic — you
  could write `digitalWrite(13, 1)` and get the same result (but don't; names are
  clearer).

---

## Gotchas

| Mistake | What happens |
|---|---|
| Forgetting `pinMode(13, OUTPUT)` | Pin defaults to INPUT; `digitalWrite` won't drive it as expected |
| `delay(1000)` thinking it's seconds | It's **milliseconds**; 1000 = 1 s, not 1000 s |
| Putting the blink in `setup()` | Runs once, never repeats — blink code belongs in `loop()` |
| External LED with no series resistor | Over-current can damage the LED and the pin |
| `digitalWrite(13, HIGH)` on an INPUT pin | Enables the internal pull-up instead of driving HIGH |

---

## TL;DR

`pinMode` picks a pin's direction, `digitalWrite` sets it HIGH (~5 V) or LOW (0 V), and
`delay` pauses everything in between. `setup()` configures pin 13 as an OUTPUT once;
`loop()` toggles it on and off with one-second pauses and is called forever by the
framework's hidden `main()`. That on/off voltage on a wire is the most basic form of
digital output — every other actuator builds on it.
