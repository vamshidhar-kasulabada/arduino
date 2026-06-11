# Random Numbers & Bit Manipulation (Challenge #7)

A companion to `07_electronic_dice.ino`. The dice rolls because of **pseudo-random
numbers**, and the result is shown on 3 LEDs using **bit manipulation**. This note
explains both from the ground up, then ties each idea back to the code you wrote.

---

## 1. Computers can't be truly random

A CPU is deterministic: same inputs, same outputs, every time. So `random()` does not
produce "real" randomness. It runs a formula (a **Pseudo-Random Number Generator**,
PRNG) that, starting from a *seed*, spits out a sequence that *looks* random but is
completely repeatable.

That repeatability is the trap: if the seed is always the same, the "random" dice
rolls the same sequence on every power-up. To break that, you seed from something
unpredictable.

```cpp
randomSeed(analogRead(A0));   // seed from a floating analog pin
```

`A0` has nothing connected to it, so it picks up electrical noise — a slightly
different value each boot. Feeding that noise into `randomSeed()` makes the PRNG start
at a different place in its sequence each time. Call this **once**, in `setup()`.

---

## 2. Generating the dice value

A die has faces 1 through 6. The Arduino `random()` function has two forms:

```cpp
random(max);        // returns 0 .. max-1
random(min, max);   // returns min .. max-1   <-- upper bound is EXCLUSIVE
```

The exclusive upper bound is the #1 gotcha. To get 1..6 you must ask for `random(1, 7)`,
**not** `random(1, 6)` (which would only ever give 1..5):

```cpp
value = random(1, 7);             // final result: 1..6
byte rand = random(1, 7);         // animation frame: 1..6
```

```
random(1, 7)  ->  range is [1, 7)  ->  {1, 2, 3, 4, 5, 6}
                                  └── 7 is never returned
```

Your sketch uses it in two places: rapid throwaway values during `ROLLING` (the
animation), and one final `value` that gets latched when the roll ends.

---

## 3. Showing a number on 3 LEDs (binary)

You have 3 LEDs but want to display values up to 6. You don't need 6 LEDs — 3 bits can
encode 0..7, and 1..6 fits comfortably. Each LED becomes one **binary digit**:

```
Value  Binary   LED3(pin10) LED2(pin9) LED1(pin8)
  1     001        OFF         OFF        ON
  2     010        OFF         ON         OFF
  3     011        OFF         ON         ON
  4     100        ON          OFF        OFF
  5     101        ON          OFF        ON
  6     110        ON          ON         OFF
       bit2        bit1        bit0
```

`LED1` (pin 8) is the **least significant bit** (the 1s place), `LED3` (pin 10) is the
**most significant bit** (the 4s place).

---

## 4. Extracting bits with `&` and `>>`

`displayValue()` is where the bit manipulation lives:

```cpp
void displayValue(uint8_t value) {
  digitalWrite(8,  value       & 1);   // bit 0
  digitalWrite(9, (value >> 1) & 1);   // bit 1
  digitalWrite(10,(value >> 2) & 1);   // bit 2
}
```

Two operators do all the work:

- **`& 1` (bitwise AND with 1)** keeps only the lowest bit and discards the rest. It
  answers "is the 1s bit set?" — giving exactly `0` or `1`, which is what
  `digitalWrite` wants (`LOW`/`HIGH`).
- **`>> n` (right shift)** slides the bits right by `n` positions, moving the bit you
  care about *into* the lowest position so `& 1` can read it.

Walk through `value = 5` (binary `101`):

```
value      = 1 0 1
value & 1            -> 1            (LED1 / pin 8  ON)

value >> 1 = 0 1 0
(value>>1) & 1       -> 0            (LED2 / pin 9  OFF)

value >> 2 = 0 0 1
(value>>2) & 1       -> 1            (LED3 / pin 10 ON)
```

Result: LEDs ON-OFF-ON = `101` = 5. The function turns any 0..7 into a 3-LED pattern
with no `if` ladder. Passing `displayValue(0)` clears all three LEDs — that's how
`WAITING` shows nothing.

> Note: `digitalWrite` treats *any* non-zero as `HIGH`, so the `& 1` is what guarantees
> a clean `0`/`1` rather than, say, the raw value `5`.

---

## 5. The state machine that drives it

The dice is a 3-state machine declared with an **enum**:

```cpp
enum State { WAITING, ROLLING, RESULT };
State game = WAITING;
```

An enum gives readable names to integers (`WAITING==0`, `ROLLING==1`, `RESULT==2`), so
`switch (game)` reads like English instead of `switch (0/1/2)`.

```
WAITING ──button press──► ROLLING ──5s elapsed──► RESULT ──10s elapsed──► WAITING
```

- **WAITING**: `displayValue(0)` — all off, watch for a press.
- **ROLLING**: every 300 ms show a fresh `random(1,7)` frame; after 5 s latch the final
  `value` and move to RESULT.
- **RESULT**: hold `value` on the LEDs for 10 s, then return to WAITING.

---

## 6. Edge detection (catching the press, not the hold)

`digitalRead(2)` is `HIGH` when released (thanks to `INPUT_PULLUP`) and `LOW` when
pressed. If you reacted to `LOW` directly, a 1-second press would fire thousands of
times. Instead you detect the **edge** — the moment it *changes* from released to
pressed:

```cpp
if (prevBtnState == HIGH && currentBtnState == LOW && game == WAITING) {
  game = ROLLING;
  ...
}
prevBtnState = currentBtnState;     // remember for next loop
```

The extra `&& game == WAITING` is what makes the code "ignore button presses while
already rolling or displaying a result," as the spec requires.

---

## 7. Non-blocking timing with `millis()`

There is no `delay()` anywhere. `millis()` returns milliseconds since boot, and you
compare timestamps:

```cpp
if (millis() - time >= 5000) { ... }       // 5 s of rolling
else if (millis() - tempTime >= 300) { ...}// 300 ms per animation frame
```

`time` marks when the roll started; `tempTime` marks the last animation frame (and is
reused as the RESULT timer). Because `loop()` never blocks, button reads and timers
stay responsive — essential on a single-threaded AVR.

> The `millis() - time` subtraction is also rollover-safe: when `millis()` wraps after
> ~49 days, unsigned arithmetic still yields the correct elapsed time.

---

## 8. Embedded relevance (AVR specifics)

- `random()` on AVR is a 32-bit linear-congruential-style PRNG; it is **not**
  cryptographically secure. Fine for games, never for keys.
- `analogRead(A0)` on an unconnected pin gives weak entropy. For better seeds people XOR
  several reads or use a dedicated noise source — but for a dice, one read is plenty.
- Bit tricks (`& `, `>>`) compile to single AVR instructions and use zero extra RAM,
  which is why register/pin packing is everywhere in firmware.

---

## Gotchas

| You write | What actually happens |
|---|---|
| `random(1, 6)` | returns 1..5 — the 6 is missing (upper bound is exclusive) |
| forgetting `randomSeed()` | same "random" sequence on every reset |
| `randomSeed()` inside `loop()` | re-seeding constantly, killing randomness — seed once in `setup()` |
| `digitalWrite(8, value >> 0)` without `& 1` | writes the whole value; works by luck since non-zero = HIGH, but unclear and bug-prone |
| reacting to `currentBtnState == LOW` directly | fires every loop the button is held; use edge detection |
| `analogRead` on a *connected* A0 | a steady voltage gives a predictable seed |

---

## TL;DR

`random(1, 7)` (seeded once with `randomSeed(analogRead(A0))`) gives an unpredictable
1..6, and `displayValue()` paints that number onto 3 LEDs by reading each bit with
`(value >> n) & 1`. Wrapped in a `WAITING → ROLLING → RESULT` enum state machine driven
by `millis()` and edge-detected button presses, those two small ideas — pseudo-random
numbers and bit manipulation — are the whole dice.
