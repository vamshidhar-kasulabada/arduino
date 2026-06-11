# Digital Input & Pull-Ups (Challenge #2)

A companion to `02_button_input.ino`. Challenge #1 *drove* a pin; this one *reads* one.
Reading a button is deceptively tricky because of a hardware problem called a **floating
pin** — this note explains `digitalRead`, why `INPUT_PULLUP` exists, and the inverted
logic that follows from it.

---

## 1. Reading a pin instead of driving it

In challenge #1 pin 13 was an OUTPUT. Here pin 8 is an INPUT — we want to *sense* the
voltage the button puts on it:

```cpp
void setup() {
  Serial.begin(9600);
  pinMode(8, INPUT_PULLUP);   // pin 8 listens, with a built-in pull-up resistor
}

void loop(){
  if (digitalRead(8) == LOW){    // pin reads 0 V  -> button is pressed
    Serial.println("HIGH");
  } else {
    Serial.println("LOW");
  }
}
```

`digitalRead(8)` returns `HIGH` (1) or `LOW` (0) depending on the voltage on pin 8.
The whole challenge is mapping that voltage to "pressed" or "released".

---

## 2. The floating-pin problem

A bare input pin is **high-impedance** — it barely draws current, so it acts like a tiny
antenna. If nothing is actively holding it at a known voltage, its reading drifts
randomly between HIGH and LOW from electrical noise. This is a **floating** pin.

A naive button wiring makes this worse:

```
   5V ──[ button ]── pin 8        (button OPEN -> pin 8 connected to nothing -> FLOATS)
```

When the button is open the pin is connected to nothing, so `digitalRead` returns
garbage. We need a resistor to gently tie the pin to a *known* level when the button is
not pressed. That resistor is a **pull-up** (ties to 5 V) or **pull-down** (ties to GND).

---

## 3. `INPUT_PULLUP` — the resistor that's already inside the chip

The AVR has a built-in pull-up resistor (~20–50 kΩ) on every pin. `INPUT_PULLUP`
switches it on, so you don't need an external resistor:

```cpp
pinMode(8, INPUT_PULLUP);   // enable the internal pull-up on pin 8
```

Now wire the button between pin 8 and **GND**:

```
   5V
    │
   [ internal pull-up ~30kΩ ]   (inside the AVR)
    │
  pin 8 ──────────┬──────► digitalRead()
                  │
              [ button ]
                  │
                 GND
```

- **Button RELEASED (open):** no path to GND. The pull-up gently pulls pin 8 up to 5 V
  -> `digitalRead` returns **HIGH**.
- **Button PRESSED (closed):** pin 8 is connected straight to GND. GND wins over the weak
  pull-up -> `digitalRead` returns **LOW**.

---

## 4. The inverted logic (and the misleading prints)

Because of the pull-up wiring, **pressed = LOW** and **released = HIGH**. This is the
opposite of what beginners expect, and it's why the `if` checks for `LOW`:

```cpp
if (digitalRead(8) == LOW){   // LOW means the button IS pressed
  Serial.println("HIGH");
}
```

| Button | Pin 8 voltage | `digitalRead(8)` |
|---|---|---|
| Released (open) | ~5 V (pulled up) | `HIGH` |
| Pressed (closed) | 0 V (shorted to GND) | `LOW` |

> Note on the strings: this sketch prints the literal text `"HIGH"` when the button is
> *pressed* (i.e. when the pin reads LOW) and `"LOW"` when released. So the printed word
> is the *inverse* of the actual pin level — it reflects the challenge's labelling, not
> the electrical reading. If you wanted the print to match the pin, you'd swap the two
> strings. Knowing this distinction is the whole point of the exercise.

---

## 5. `Serial` — talking back to your computer

```cpp
Serial.begin(9600);          // open the serial port at 9600 baud
Serial.println("HIGH");      // send a line of text + newline
```

- `Serial.begin(9600)` configures the UART hardware to a **baud rate** of 9600 bits/sec.
  The number in your Serial Monitor must match this exactly, or you'll see gibberish.
- `Serial.println(...)` sends text over USB to the Serial Monitor and appends a newline
  (`\n`), so each reading lands on its own line. `Serial.print(...)` is the same without
  the newline.
- `begin` belongs in `setup()` (configure once); the printing happens in `loop()`.

This `loop()` prints on **every iteration** with no delay, so the monitor scrolls very
fast. That's fine for learning, but it's why later challenges only print on *changes*.

---

## 6. The C++ / embedded angle

- `digitalRead` returns an `int` that is one of the constants `HIGH` (1) or `LOW` (0), so
  comparing with `== LOW` is comparing against `0`.
- `INPUT_PULLUP` vs `INPUT`: both make the pin an input, but `INPUT` leaves the pin
  floating (you must add your own external resistor), while `INPUT_PULLUP` enables the
  internal one. Prefer `INPUT_PULLUP` for buttons — fewer parts, fewer bugs.
- Under the hood, enabling the pull-up writes a `1` to the pin's bit in the `PORT`
  register *while* the `DDR` bit says input. Arduino hides this behind one tidy constant.

---

## Gotchas

| Mistake | What happens |
|---|---|
| Using plain `INPUT` with no external resistor | Pin floats -> random HIGH/LOW readings |
| Expecting pressed == HIGH | With a pull-up, pressed == **LOW** (inverted) |
| Serial Monitor baud ≠ `9600` | Garbled / unreadable output |
| Calling `Serial.begin` in `loop()` | Re-initialises the port every iteration; put it in `setup()` |
| Wiring the button to 5 V instead of GND | Breaks the pull-up scheme; pin no longer reads LOW on press |

---

## TL;DR

An input pin left alone **floats** and reads noise. `INPUT_PULLUP` switches on the AVR's
internal resistor so the pin sits at a known HIGH when the button is open, and drops to
LOW when the button shorts it to GND. That's why this sketch tests `digitalRead(8) ==
LOW` to detect a press — the logic is inverted by design. `Serial.begin(9600)` plus
`Serial.println` give you a window into what the pin is reading.
