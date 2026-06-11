# Pointers, Array Decay & References (Challenge #20)

A companion to `20_dynamic_led_sequence.ino`. The goal of this challenge is to write
**one** function that animates *any* number of LEDs — 3, 5, or 10 — without changing a
line. The thing that makes that possible is how arrays turn into pointers when you pass
them. Read this next to your code; everything uses your real `leds[]`, `updateSequence`,
and `size`.

---

## 1. A pointer is just an address

Every variable lives at some address in memory. A **pointer** is a variable that stores
*an address* instead of a value. The `*` in a declaration means "pointer to":

```cpp
int x = 42;
int *p = &x;   // p holds the ADDRESS of x   (& = "address of")
*p;            // 42   (* = "value at that address" — "dereference")
```

So `p` doesn't contain `42`; it contains "where to find the 42." That indirection is the
whole point — it lets a function reach data that lives somewhere else.

---

## 2. Your array and how it sits in memory

```cpp
Led leds[] = {8, 9, 10};
int size = sizeof(leds) / sizeof(leds[0]);   // 3
```

`leds` is an array of three `Led` objects, contiguous in RAM:

```
leds[]:  ┌ leds[0] ┐┌ leds[1] ┐┌ leds[2] ┐
         │  Led(8) ││  Led(9) ││ Led(10) │
         └─────────┘└─────────┘└─────────┘
         ▲
         └─ the array's name refers to this whole block,
            and its address is the address of leds[0]
```

`sizeof(leds)` is the size of the **whole array** (3 × `sizeof(Led)`), and
`sizeof(leds[0])` is one element — so the division gives the element **count, 3**. This
trick only works here, in `loop`'s scope, where `leds` is still a real array (keep
reading to see why that's a caveat).

---

## 3. Array decay: `Led leds[]` becomes `Led*` when passed

Here is the central idea of the challenge. Look at your function signature:

```cpp
void updateSequence(Led *leds, const int size, int position) { ... }
```

You call it with the array:

```cpp
updateSequence(leds, size, position);
```

C++ does **not** copy the array into the function. Instead the array name **decays** to a
pointer to its first element. These two signatures are *identical* to the compiler:

```cpp
void updateSequence(Led leds[], ...)   // looks like an array...
void updateSequence(Led *leds,  ...)   // ...is secretly THIS
```

Visually, the decay:

```
caller:                          inside updateSequence:

leds[]: [Led][Led][Led]          leds  ──►  [Led][Led][Led]
        ▲                        (a Led*)    ▲
        └─ real array                        └─ points back at the SAME memory
```

The function receives only an **address**, not the elements. That's why:

- It's cheap — passing 3 LEDs and passing 100 LEDs both copy one pointer (2 bytes on
  AVR).
- It operates on the **original** LEDs — `leds[i].on()` inside the function lights the
  real hardware, not a copy.

---

## 4. The catch decay creates: the size is *lost*

Because the function only gets a pointer, it has **no idea how long** the array is.
`sizeof(leds)` *inside* `updateSequence` would give the size of a **pointer** (2 bytes),
not the array — useless for counting.

```cpp
// INSIDE the function, this would be WRONG:
int n = sizeof(leds) / sizeof(leds[0]);   // = sizeof(Led*) / sizeof(Led) — nonsense
```

That is exactly why the challenge requires you to pass `size` as a **separate
parameter**. The "pointer + count" pair is the universal C/C++ idiom for handing an
array across a function boundary:

```cpp
void updateSequence(Led *leds, const int size, int position)
//                  └─ where ─┘  └── how many ──┘
```

`const int size` marks the count as read-only inside the function — a small promise that
the function won't try to change how many LEDs there are.

---

## 5. The animation logic (why it's generic)

```cpp
void updateSequence(Led *leds, const int size, int position) {
  for (int i = 0; i < size; i++) {
    if (i == position) {
      leds[i].on();      // the one LED that should be lit
    } else {
      leds[i].off();     // every other LED off
    }
  }
}
```

Nothing here mentions pin 8, 9, or 10, or the number 3. It loops `0 .. size-1` and lights
whichever index equals `position`. Feed it 10 LEDs and `size = 10` and it just works —
that's the "no hardcoded indexes" rule paying off. `leds[i]` is pointer indexing:
`leds[i]` means "the element `i` slots past where `leds` points" (identical to
`*(leds + i)`).

The caller drives `position` forward and wraps it:

```cpp
if (position == size) { position = 0; }            // wrap back to the start
if (millis() - timer >= interval) {
  updateSequence(leds, size, position);
  timer += interval;
  position++;
}
```

Non-blocking `millis()` timing (no `delay()`), so the "moving dot" advances every
`interval` ms while the CPU stays free.

---

## 6. References — the other way to pass without copying

The challenge lists **references** as a practiced concept. A reference is an **alias**:
another name for an existing variable. You declare it with `&`, and you use it with no
special syntax (no `*` to dereference):

```cpp
void brighten(Led &led) { led.on(); }   // led is an ALIAS for the caller's Led
Led d13(13);
brighten(d13);                            // operates on the real d13
```

Pointer vs reference, side by side:

```cpp
void f(Led *led) { led->on(); }   // pointer: may be null, can be repointed, use -> / *
void g(Led &led) { led.on();  }   // reference: must bind to a real object, use .
```

| | Pointer (`Led*`) | Reference (`Led&`) |
|---|---|---|
| Can be null / empty? | Yes | No — must alias something real |
| Can be reassigned? | Yes (`p = &other`) | No — bound for life |
| Member access | `led->on()` or `(*led).on()` | `led.on()` |
| Arrays decay to it? | **Yes** (`Led[]` → `Led*`) | No |

For a *single* object, a reference is the cleaner "no-copy" tool (you saw it in #19's
`for (Task &task : tasks)`). For an **array**, decay hands you a **pointer** — which is
why `updateSequence` takes `Led *leds`. Both share the same purpose: act on the caller's
real data instead of a copy.

---

## 7. Embedded angle: reusable drivers

This is how generic hardware drivers are written. A function (or library) that takes
`(thing*, count)` doesn't care whether you wired up 3 LEDs or a 16-LED bar — the same
binary code drives all of them. You separate **logic** ("light index `position`") from
the **specific hardware instance** (this particular array on these pins). On a tiny AVR
that also saves flash: one function, not three copy-pasted ones.

---

## 8. The bridge to Challenge 21

Right now `leds`, `size`, and `position` are loose globals and you pass them into a free
function each tick. Challenge #21 takes the next step: hand the array pointer + count to
a **class** (`Scheduler`) *once*, in its constructor, and let the object remember them.
Same "pointer + count" idea — now owned by a reusable component instead of floating in
`loop()`.

---

## Gotchas

| Symptom | Cause |
|---|---|
| `sizeof(leds)/sizeof(leds[0])` gives 1 (or garbage) inside the function | `leds` decayed to a pointer — `sizeof` measures the pointer, not the array. Pass `size` explicitly. |
| Function changes don't affect real LEDs | You didn't pass a pointer/reference (or copied the objects) — pass `Led*` so it acts on the originals |
| Reads one past the end | Loop must be `i < size`, and `position` must wrap with `if (position == size) position = 0;` |
| Crash / wrong LED with `Led*` | Pointer not pointing at a valid array, or `size` larger than the real array |
| Tried `Led &leds[]` as a param | You can't have an array-of-references param; arrays decay to pointers — use `Led *leds` |

---

## TL;DR

Passing an array doesn't copy it: the array name **decays** to a pointer to its first
element, so `updateSequence(Led *leds, ...)` receives just an address and works on the
**real** LEDs. Decay also throws away the length, so you pass `size` alongside the pointer
— the universal "pointer + count" idiom. References (`Led&`) are the no-copy tool for a
*single* object; pointers (`Led*`) are what you get for *arrays*. Together they let one
generic function drive any number of LEDs.
