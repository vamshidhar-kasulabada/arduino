# Arrays of Objects (Challenge #11)

A companion to `11_led_array_scanner.ino`. The Knight Rider / Cylon scanner in this
challenge works by storing **objects in an array** and indexing into them instead of
hard-coding pin numbers. This note explains how an array of `Led` objects is built,
laid out in memory, and driven by a small state machine.

---

## 1. From pins to objects

In earlier sketches you thought in **pins**:

```cpp
digitalWrite(8, HIGH);
digitalWrite(9, LOW);
```

Here you think in **objects**:

```cpp
leds[0].on();
leds[1].off();
```

The `Led` class wraps a pin plus its on/off state, exposing a clean API (`on()`,
`off()`, `toggle()`, `isOn()`). The rules forbid calling `digitalWrite()` or `pinMode()`
outside the class — every pin operation must go through an `Led` method. The array is
what lets you treat three of these objects uniformly.

---

## 2. Declaring an array of objects (with constructor args)

A normal array holds values. An **array of objects** holds fully-constructed instances —
and because `Led` has a constructor that *requires* a pin, you must supply one per
element:

```cpp
Led leds[] = {Led(8), Led(9), Led(10)};
```

Each `Led(8)`, `Led(9)`, `Led(10)` runs the constructor:

```cpp
Led(uint8_t p) {
  pin = p;
  state = false;
  pinMode(pin, OUTPUT);   // <-- runs once per element, at construction
  write();                // drives the pin LOW immediately
}
```

So just by *declaring the array*, all three pins get configured as `OUTPUT` and driven
LOW. There is no separate setup loop — `setup()` is empty. The objects configured the
hardware as they were born. (These are **global** objects, so their constructors run
before `setup()` even starts.)

Because there's no default `Led()` constructor (the only one needs a pin), you **must**
brace-initialize every slot — you can't write `Led leds[3];` and fill them later.

---

## 3. Computing the size — don't hard-code 3

```cpp
uint8_t size = sizeof(leds) / sizeof(leds[0]);
```

`sizeof(leds)` is the total bytes of the whole array; `sizeof(leds[0])` is the bytes of
one `Led`. Dividing gives the element **count** (3 here). Write the array literal once
and this stays correct if you add a fourth LED — no magic number to forget.

```
  sizeof(leds)            = 3 objects worth of bytes
  ──────────────────────────────────────────────────  = 3
  sizeof(leds[0])         = 1 object's bytes
```

---

## 4. Memory layout

The objects sit **contiguously** in memory, back to back. Each `Led` holds a `uint8_t pin`
(1 byte) and a `bool state` (1 byte):

```
        leds[0]        leds[1]        leds[2]
      ┌───────────┐  ┌───────────┐  ┌───────────┐
      │ pin  = 8  │  │ pin  = 9  │  │ pin  = 10 │
      │ state= ?  │  │ state= ?  │  │ state= ?  │
      └───────────┘  └───────────┘  └───────────┘
      ^ base         ^ base+stride  ^ base+2*stride
```

`leds[i]` is computed as *base address + i × sizeof(Led)* — pure pointer arithmetic, so
indexing is O(1). This contiguity is exactly why `sizeof` division gives the count, and
why you can hand the array to a function as a pointer (next section).

---

## 5. Passing the array to a function

The helper that lights exactly one LED takes the array, the index to turn on, and the
size:

```cpp
void onLed(Led leds[], uint8_t ledToOn, uint8_t size) {
  for (int i = 0; i < size; i++) {
    if (i == ledToOn) {
      leds[i].on();
    } else {
      leds[i].off();
    }
  }
}
```

Important C++ subtlety: **`Led leds[]` as a parameter is really `Led* leds`** — arrays
*decay* to pointers when passed. The function receives the address of the first element,
not a copy of all three objects. That's why mutating `leds[i]` inside `onLed` affects the
real LEDs, and it's why you must pass `size` separately — the function can no longer use
`sizeof` to recover the count (it would just get the size of a pointer).

This loop enforces the "only one LED on at a time" requirement: it turns the chosen LED
`on()` and every other one `off()` on each tick.

---

## 6. The scanner state machine

Two pieces of state drive the motion, plus a non-blocking timer:

```cpp
int led = 0;                  // which LED is currently lit
Direction direction = RIGHT;  // which way the dot is moving
unsigned long time = 0;       // last-update timestamp
```

`Direction` is an `enum` giving names to the two directions:

```cpp
enum Direction { RIGHT, LEFT };
```

Every 300 ms, advance one step and bounce off the ends:

```cpp
if (millis() - time >= 300) {
  time = millis();
  onLed(leds, led, size);
  switch (direction) {
  case RIGHT:
    led++;
    if (led == size) {        // ran off the right end
      direction = LEFT;
      led = size - 2;         // step back to the second-from-last
    }
    break;
  case LEFT:
    led--;
    if (led == -1) {          // ran off the left end
      direction = RIGHT;
      led = 1;                // step forward to the second element
    }
    break;
  }
}
```

The `size - 2` and `1` corrections are what make the bounce smooth instead of repeating
the end LED twice:

```
  index:   0   1   2
  RIGHT →  ●   ○   ○
           ○   ●   ○
           ○   ○   ●   led hits size(3) -> flip to LEFT, led = size-2 = 1
  LEFT  ←  ○   ●   ○
           ●   ○   ○   led hits -1 -> flip to RIGHT, led = 1
           ○   ●   ○   ... repeats forever
```

Note `led` is a signed `int` precisely so the `led == -1` underflow test works — if it
were an unsigned type, `0--` would wrap to a huge positive number and the comparison
would never match.

---

## 7. Embedded relevance

- **One construction step, many objects.** Declaring `leds[]` configured three output
  pins via three constructor calls — scaling to 8 LEDs means editing one line.
- **Uniform iteration.** Hardware abstracted as objects lets a single `for` loop drive
  any number of LEDs; the algorithm doesn't care which pins they're on.
- **Non-blocking timing.** `millis() - time >= 300` paces the animation without
  `delay()`, so `loop()` stays free for other work (buttons, serial, etc.).
- **RAM cost.** Each object is tiny here (~2 bytes of fields), but on an AVR with only
  2 KB of SRAM, arrays of *large* objects add up — worth keeping in mind as classes grow.

---

## Gotchas

| Pitfall | What happens | Fix |
|---|---|---|
| `Led leds[3];` with no initializer | Won't compile — no default constructor | Brace-init every slot: `{Led(8), Led(9), Led(10)}` |
| Hard-coding `size = 3` | Breaks when you add/remove an LED | `sizeof(leds)/sizeof(leds[0])` |
| Using `sizeof` *inside* `onLed` | Array decayed to a pointer; gives pointer size | Pass `size` as a parameter |
| Making `led` unsigned | `led == -1` never true; index wraps huge | Keep `led` a signed `int` |
| Forgetting the `size-2` / `1` fixups | End LED flashes twice on each bounce | Step back one when reversing |
| Calling `digitalWrite` directly | Breaks the abstraction (and the rules) | Always go through `led.on()/off()` |

---

## TL;DR

`Led leds[] = {Led(8), Led(9), Led(10)};` builds an **array of objects** — three
fully-constructed `Led`s laid out contiguously in memory, each having configured its own
pin in its constructor. Indexing (`leds[i]`) is O(1) pointer math, and passing the array
to `onLed()` decays it to a pointer (so you also pass `size`). A tiny `direction` + `led`
state machine, paced by `millis()`, walks the lit index back and forth — turning three
pins into a reusable, scalable scanner with no raw `digitalWrite` in sight.
