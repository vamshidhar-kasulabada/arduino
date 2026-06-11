# Reading Inputs & Pull-Up Resistors — Registers, Part 2 (Challenge #26)

A companion to `26_register_button_counter.ino`. Challenge 25 covered the **output** side of
port manipulation (set a pin's direction, drive it HIGH/LOW). This is the **input** side:
how to *read* a pin and how to stop an input from reading garbage. It assumes you've read
`../25_port_manipulation/registers_and_bitwise.md` — the operators (`& | ^ ~ << >>`), masks,
the four idioms, and read-modify-write are reused here without re-explaining.

---

## 0. New terms in this challenge

| Term | Plain meaning |
|---|---|
| **input pin** | a pin set to *listen* to a voltage, not drive one (`DDR` bit = 0) |
| **floating** | an input pin connected to nothing — its reading is random electrical noise |
| **pull-up resistor** | a resistor that gently ties a pin to **HIGH (5 V)** when nothing else drives it |
| **pull-down resistor** | same idea, but ties the pin to **LOW (0 V)** |
| **internal pull-up** | a pull-up resistor *built into the chip*, switched on in software (no wiring) |
| **active-low** | "pressed/active = `0`, idle = `1`" — the *opposite* of what feels natural |
| **edge** | the *moment* a signal changes: rising = `0→1`, falling = `1→0` |
| **debounce** | ignoring the fast on/off chatter a mechanical switch makes when it's pressed |

---

## 1. Same three registers, read the other way

You already met Port B's three registers. Here's the **whole** picture — both directions:

| Register | When the pin is an **output** (`DDR`=1) | When the pin is an **input** (`DDR`=0) |
|---|---|---|
| `DDRB`  | `1` = output | `0` = input |
| `PORTB` | `1` = drive HIGH, `0` = drive LOW | `1` = **enable internal pull-up**, `0` = no pull-up |
| `PINB`  | (reads back what you drove) | **read the live pin level here** |

Two things to burn in:

1. **You READ from `PINB`, not `PORTB`.** `PORTB` is what you *want* the pin to be (output)
   or whether the pull-up is on (input). `PINB` is what the pin *actually is* right now.
2. **On an input pin, writing `PORTB` does NOT drive the pin** — it switches the **pull-up**
   on/off. Same register, completely different job, decided by the `DDR` bit.

---

## 2. Why a bare input "floats" (and why that's bad)

An input pin is extremely high-resistance — it barely draws any current, it just *senses*
voltage. If you connect a button between the pin and GND like this:

```
   pin ----[ button ]---- GND
```

then while the button is **pressed**, the pin is connected to GND → reads `0`. Clear enough.
But while the button is **released**, the pin is connected to *nothing*. It's **floating** —
it picks up stray voltage from nearby wires, your hand, the mains hum in the air — and reads
`0` or `1` at random. Your button would appear to press itself.

A pin must always have a *defined* level when nothing is actively driving it. That's the
resistor's job.

---

## 3. The pull-up resistor: a default of HIGH

A **pull-up** is a resistor from the pin to **+5 V**. It's weak (high resistance), so:

```
        +5V
         |
       [ R ]            <- pull-up resistor (weak)
         |
   pin --+----[ button ]---- GND
```

- **Button released:** no path to GND. The weak resistor quietly pulls the pin up to **5 V →
  reads 1.**
- **Button pressed:** the pin is connected straight to GND. GND wins over the weak resistor,
  so the pin goes to **0 V → reads 0.**

So with a pull-up + button-to-GND, the logic is **active-low**:

```
   released  ->  1
   pressed   ->  0      <-- "active" (the thing you care about) is the LOW one
```

(You *could* instead use a **pull-down** resistor + button-to-+5V to get the "natural"
pressed = 1. But every AVR pin has a built-in pull-**up** and none built-in pull-down, so
pull-up + active-low is the standard, zero-extra-parts way. It's exactly what `INPUT_PULLUP`
does.)

---

## 4. The internal pull-up — switched on with registers

You don't need to solder a resistor: the ATmega has one **inside** each pin. You turn it on
in software. The rule for **input + pull-up** is:

```cpp
DDRB  &= ~(1 << PB4);   // direction bit = 0  -> PB4 is an INPUT
PORTB |=  (1 << PB4);   // PORT bit      = 1  -> internal pull-up ON
```

| `DDRB` bit | `PORTB` bit | Result for the pin |
|---|---|---|
| 0 | 0 | input, **floating** (avoid) |
| 0 | 1 | input, **pull-up on** ← what we want |
| 1 | 0 | output, driving LOW |
| 1 | 1 | output, driving HIGH |

That two-line combo *is* `pinMode(12, INPUT_PULLUP)`. Now you know what that macro was doing
all along.

> Note: clearing the `DDRB` bit uses `&= ~(1<<n)` (the CLEAR idiom). On a fresh boot all
> bits are already 0, so the clear is belt-and-suspenders — but write it anyway so the code
> is correct even if PB4 was an output earlier.

---

## 5. Reading the pin: `PINB`

To get PB4's live level, read `PINB` and slide that bit down to position 0:

```cpp
uint8_t level = (PINB >> BTN_BIT) & 1;   // BTN_BIT == PB4 == 4
//               └ shift bit 4 to bit 0 ┘ └ keep only that bit ┘
//   level == 1  -> released
//   level == 0  -> pressed   (active-low!)
```

`(PINB >> n) & 1` is the **read** idiom from Challenge 25 — same tool, now on the input
register.

---

## 6. Don't act on the *level* — act on the *edge*

If you incremented the counter whenever `level == 0`, then *holding* the button would make
the counter race upward many times per second. You want **one count per press**. So watch
for the **falling edge** — the single moment it goes from released (`1`) to pressed (`0`):

```cpp
if (lastStableLevel == 1 && newStableLevel == 0) {
    // a fresh press just happened — increment exactly once
}
lastStableLevel = newStableLevel;   // remember for next time
```

This is the same edge-detection idea as Challenge 3 (`03_led_toggle`), just reading a
register instead of `digitalRead`.

---

## 7. Debounce: a switch is messier than you think

A mechanical button doesn't go cleanly `1 → 0`. Its metal contacts physically *bounce* for a
few milliseconds, so the pin reads something like `1 1 0 1 0 0 1 0 0 0 0`. Without care,
each of those flickers looks like its own press, and one push counts as five.

**Debounce** = only believe a level once it has held steady for a short window:

```cpp
uint8_t level = (PINB >> BTN_BIT) & 1;

if (level != lastReadLevel) {       // the raw reading just changed...
    lastChangeAt = millis();        // ...restart the steadiness timer
    lastReadLevel = level;
}

if (millis() - lastChangeAt >= DEBOUNCE_MS) {
    // 'level' has been stable long enough — NOW treat it as the real level,
    // run the edge check from §6 against lastStableLevel, then store it.
}
```

`DEBOUNCE_MS` of ~20–50 ms is plenty: longer than the bounce, shorter than a human press.
(This mirrors the `Button` class from Challenge 10 — same principle, done by hand.)

---

## 8. Putting it on the LEDs without disturbing the button

The increment + display is pure Challenge-25 read-modify-write. Note the button bit (PB4)
lives in the *same* register you're writing, so you **must** mask or you'd stomp the pull-up:

```cpp
counter = (counter + 1) & 7;                         // wrap 0..7 (low 3 bits)
PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);  // change only PB0..PB2
//       └ keep PB4's pull-up + the rest ┘
```

`(counter + 1) & 7` is a neat trick: `& 7` keeps only the low 3 bits, so `8 & 7 == 0` —
the wrap-around happens for free without an `if`.

---

## 9. How this maps to the challenge

```cpp
const uint8_t LED_MASK = (1<<PB0)|(1<<PB1)|(1<<PB2);  // pins 8,9,10 out
const uint8_t BTN_BIT  = PB4;                          // pin 12 in

// setup
DDRB  |=  LED_MASK;            // LEDs are outputs
DDRB  &= ~(1 << BTN_BIT);      // button is input
PORTB |=  (1 << BTN_BIT);      // ...with pull-up on

// loop  (after debounce settles)
uint8_t level = (PINB >> BTN_BIT) & 1;        // 1 released, 0 pressed
if (wasReleased && nowPressed) {              // falling edge
    counter = (counter + 1) & 7;
    PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);
}
```

---

## 10. Gotchas cheat-sheet

| Symptom | Cause |
|---|---|
| Button "presses itself" randomly | pull-up not enabled — pin is floating (`PORTB` bit left 0) |
| Reads always the same | read `PORTB` instead of `PINB`; `PINB` is the live level |
| Logic feels backwards | it's **active-low** — pressed = `0`, released = `1` |
| One push counts many times | no debounce, or acting on level not edge |
| Holding the button keeps counting | acting on `level==0` instead of the `1→0` edge |
| LEDs change but pull-up dies | wrote `PORTB` as a whole byte — mask, don't clobber PB4 |
| Counter skips / never wraps | use `(counter + 1) & 7` (or `if (counter > 7) counter = 0`) |

---

## 11. TL;DR

Reading a pin uses the **same three registers**, the other way: `DDR` bit `0` = input, then
read the live level from **`PINB`** (not `PORTB`). A bare input **floats** and reads noise, so
enable the **internal pull-up** with `PORTB |= (1<<n)` on an input pin — that's literally what
`INPUT_PULLUP` is (`DDR=0, PORT=1`). With a pull-up the button is **active-low**: released `1`,
pressed `0`. Read it with `(PINB >> n) & 1`, **debounce** it with `millis()`, act on the
**falling edge** so one push = one count, and show the counter with **read-modify-write** so
you never disturb the button's bit. You now know *both* directions of port manipulation.
