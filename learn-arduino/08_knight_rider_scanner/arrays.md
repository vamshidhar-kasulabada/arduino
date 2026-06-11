# Arrays & Indexing (Challenge #8)

A companion to `08_knight_rider_scanner.ino`. The bouncing "Cylon eye" works because the
LED pins live in an **array** and a moving **index** walks back and forth across it. This
note explains arrays from the ground up, then ties each idea back to the code you wrote.

---

## 1. What an array is

An array is a row of variables of the same type, stored **contiguously** in memory, that
share one name and are reached by a numeric **index** starting at `0`:

```cpp
uint8_t leds[] = {8, 9, 10};
```

```
 index:    0     1     2
         ┌─────┬─────┬─────┐
 leds:   │  8  │  9  │ 10  │     <- the pin numbers
         └─────┴─────┴─────┘
 addr:   100   101   102        (1 byte each: uint8_t)
```

`leds[0]` is `8`, `leds[1]` is `9`, `leds[2]` is `10`. The name `leds` by itself is the
address of the first element — that detail matters in section 4.

Storing pins in an array is the whole point of this challenge: instead of three hard
variables (`pin8`, `pin9`, `pin10`) you have one structure you can *loop over*. Adding
LEDs 4 and 5 means changing one line — the logic doesn't care how many there are.

---

## 2. Computing the length without hard-coding it

You never wrote `size = 3`. Instead:

```cpp
uint8_t size = sizeof(leds) / sizeof(leds[0]);
```

`sizeof` reports byte counts:

- `sizeof(leds)` = size of the **whole** array = 3 elements × 1 byte = `3`.
- `sizeof(leds[0])` = size of **one** element = `1` byte.
- `3 / 1` = `3` elements.

```
 sizeof(leds)      = 3 bytes  (entire array)
 sizeof(leds[0])   = 1 byte   (single element)
 --------------------------------
 3 / 1             = 3        (element count)
```

This idiom is the canonical way to get an array's length in C/C++ and is self-updating:
expand to `{8, 9, 10, 11, 12}` and `size` becomes `5` automatically. (It only works where
`leds` is a *real array*, not a pointer — see the gotcha in section 4.)

---

## 3. Looping over the array to light one LED

`onLed()` walks every element and turns exactly one on:

```cpp
void onLed(const uint8_t leds[], uint8_t ledToOn, uint8_t size) {
  for (int i = 0; i < size; i++) {
    if (i == ledToOn) {
      digitalWrite(leds[i], HIGH);   // the chosen LED
    } else {
      digitalWrite(leds[i], LOW);    // everything else off
    }
  }
}
```

This guarantees "only one LED on at a time": each call clears all LEDs and re-lights just
the one whose index equals `ledToOn`. `i` is the index (`0..size-1`); `leds[i]` is the pin
number at that index. The classic loop shape is `for (i = 0; i < size; i++)` — note `<`,
not `<=` (section 7).

`setup()` uses the modern **range-based for** loop, which hides the index entirely:

```cpp
for (uint8_t pin : leds) {   // pin takes each value: 8, then 9, then 10
  pinMode(pin, OUTPUT);
}
```

---

## 4. Arrays "decay" to pointers when passed

Notice `onLed` takes a `size` argument even though we have `sizeof`. That's because when
you pass an array to a function it **decays** to a pointer to its first element — the
length information is lost:

```cpp
void onLed(const uint8_t leds[], ...)  // really receives a uint8_t* — NOT the array
```

Inside `onLed`, `sizeof(leds)` would give the size of a *pointer* (2 bytes on AVR), not
the array — so you'd compute the wrong length. That's why the caller measures `size` in
the global scope and passes it along:

```cpp
onLed(leds, led, size);   // hand over the data AND its length
```

The `const` says "this function won't modify the array," documenting intent and letting
the compiler catch accidental writes.

---

## 5. The scanner: an index that bounces

The animation is a single index, `led`, that increments, hits a wall, and reverses. The
direction is tracked with an enum:

```cpp
enum Direction { RIGHT, LEFT };
int led = 0;
Direction direction = RIGHT;
```

`led` is `int` (not `uint8_t`) on purpose — it must be able to hold `-1` momentarily
(section 7). Each tick:

```cpp
switch (direction) {
case RIGHT:
  led++;
  if (led == size) {        // walked off the right end (index 3 doesn't exist)
    direction = LEFT;
    led = size - 2;         // bounce back to index 1
  }
  break;
case LEFT:
  led--;
  if (led == -1) {          // walked off the left end
    direction = RIGHT;
    led = 1;                // bounce forward to index 1
  }
  break;
}
```

Why `size - 2` and `1` instead of `size - 1` and `0`? To avoid lighting the end LED
**twice in a row** (which would look like a pause at the edges). When `led` reaches the
last valid index and tries to step past it, you skip straight to the *second-to-last*:

```
indices for size=3:   0   1   2
RIGHT:  0 -> 1 -> 2 -> (hit 3, illegal) -> reverse, jump to 1
LEFT:   1 -> 0 -> (hit -1, illegal) -> reverse, jump to 1
sequence shown: 0,1,2,1,0,1,2,1,0,...  smooth bounce, no repeats at the ends
```

---

## 6. Non-blocking timing

The scanner steps every 300 ms using `millis()`, never `delay()`:

```cpp
if (millis() - time >= 300) {
  time = millis();
  onLed(leds, led, size);
  // ...advance index...
}
```

`time` is the timestamp of the last step. The subtraction is unsigned and rollover-safe.
Because `loop()` never blocks, the sketch could read buttons or sensors between steps —
the foundation of responsive firmware.

---

## 7. Boundary conditions (the part that bites everyone)

A `size`-element array has valid indices `0 .. size-1`. Reading `leds[size]` (here
`leds[3]`) is **out of bounds** — on an AVR that's not a crash but a silent read of
whatever byte sits next in RAM. The bounce logic exists precisely to *detect* the moment
the index would go out of range (`led == size` or `led == -1`) and pull it back **before**
that bad index is ever passed to `onLed()`.

This is also why `led` must be a signed `int`: the `LEFT` case lets it reach `-1` to test
the lower edge. A `uint8_t` would wrap `0 - 1` to `255` and the `== -1` test would never
be true.

---

## Gotchas

| Pitfall | What goes wrong |
|---|---|
| `for (i = 0; i <= size; i++)` | `<=` reads index `size` — one past the end (off-by-one) |
| hard-coding `size = 3` | breaks the moment you add an LED; use `sizeof(arr)/sizeof(arr[0])` |
| `sizeof` *inside* a function on a passed array | array decayed to a pointer → wrong length; pass `size` explicitly |
| `uint8_t led` in the bounce | `0 - 1` wraps to 255, so `led == -1` never fires; use signed `int` |
| reversing at `size-1` / `0` | end LEDs light twice in a row; reverse to `size-2` / `1` for a clean bounce |
| accessing `leds[size]` | out-of-bounds; reads garbage RAM, no crash to warn you |

---

## TL;DR

The pins live in one array, `leds[]`, sized at compile time by
`sizeof(leds)/sizeof(leds[0])`. A single index, `led`, walks across it; `onLed()` loops
the array and lights only `leds[led]`. The `RIGHT`/`LEFT` enum plus the careful boundary
checks (`led == size`, `led == -1`, bouncing to `size-2`/`1`) keep that index in range and
make the eye sweep smoothly. Arrays + indexing + boundary checks — that's the entire
Knight Rider effect.
