# Non-Blocking Timing with `millis()` (Challenge #4)

A companion to `04_timed_led.ino`. The whole point of this challenge is to keep an LED
ON for 5 seconds **without freezing the program** — this note explains how `millis()`
makes that possible, from the ground up, tied to the code you wrote.

---

## 1. The problem with `delay()`

The "obvious" way to keep an LED on for a while is:

```cpp
digitalWrite(13, HIGH);
delay(5000);            // <-- the trap
digitalWrite(13, LOW);
```

`delay()` is **blocking**: while it counts down, the CPU does *nothing else*. It cannot
read the button, cannot blink a second LED, cannot answer Serial. For 5 whole seconds
your sketch is deaf and dumb. That is why the challenge explicitly bans it:

> Requirements: Use `millis()`. Do not use `delay()`.

The cure is to stop *waiting* and start *checking the clock*.

---

## 2. What `millis()` actually is

`millis()` returns the number of **milliseconds since the board powered on**, as an
`unsigned long` (32-bit on the Uno's ATmega328P). It never blocks — it just reads a
counter that a hardware timer (Timer0) increments in the background.

```
power-on                          now
   |================================|
   0 ms                          millis()  ->  e.g. 41873
```

You don't *wait* for time to pass. You record a timestamp, then on every `loop()` you
ask "how long since then?" That is the non-blocking mindset.

---

## 3. The "record a start, measure elapsed" pattern

Your sketch stores the moment the button was pressed:

```cpp
unsigned buttonLastPressedAt = 0;
```

When the press is detected, you stamp it:

```cpp
buttonLastPressedAt = millis();   // remember WHEN it happened
ledState = HIGH;
```

Then, every single loop, you check whether enough time has elapsed:

```cpp
if (millis() - buttonLastPressedAt >= time) {   // time == 2000 ms here
  ledState = LOW;
}
```

`millis() - buttonLastPressedAt` is the **elapsed time** since the press. The moment it
reaches `time`, the LED turns off — but in the meantime `loop()` keeps spinning freely,
so the button is still being read. That is the entire trick.

> Note: the header comment says "5 seconds," but the code uses `uint16_t time = 2000;`
> — i.e. 2 s. To actually match the objective you'd set `time = 5000;`. (A `uint16_t`
> tops out at 65535, so 5000 still fits fine.)

---

## 4. Why `millis() - start >= interval` and not `millis() >= start + interval`

This idiom is **overflow-safe**, and that matters because `millis()` *will* overflow.

An `unsigned long` is 32 bits, so its max value is 4,294,967,295 ms ≈ **49.7 days**.
After that it wraps back to 0. The beauty of unsigned arithmetic is that **subtraction
still gives the correct elapsed time across the wrap**:

```
Say interval = 2000.
start    = 4294966000      (just before overflow)
millis() =          1000   (already wrapped past 0)

millis() - start  =  1000 - 4294966000   (unsigned!)
                  =  3296   (wraps around correctly)  -> elapsed = 3296 ms  ✓
```

Because the math is done in unsigned (modulo 2^32) arithmetic, the difference is always
the true gap. Compare that to the *broken* version:

```cpp
if (millis() >= buttonLastPressedAt + time)   // BUG near overflow
```

Here `buttonLastPressedAt + time` can itself overflow and become a tiny number, making
the condition fire immediately or never. **Always subtract, then compare.**

| Idiom | Overflow-safe? |
|---|---|
| `millis() - start >= interval` | ✅ yes — use this |
| `millis() >= start + interval` | ❌ no — breaks near the 49.7-day wrap |

---

## 5. Detecting the press: edge detection

The LED should restart its 5-second window only on a **fresh press**, not continuously
while the button is held. That requires detecting the *transition*, not the *level*:

```cpp
int prevPin8state = HIGH;
...
int currentPin8State = digitalRead(8);
if (prevPin8state == HIGH && currentPin8State == LOW) {   // HIGH -> LOW edge
  buttonLastPressedAt = millis();
  ledState = HIGH;
}
prevPin8state = currentPin8State;                          // remember for next loop
```

You compare *this* loop's reading against *last* loop's. A `HIGH -> LOW` change is the
"falling edge" — the instant the button goes down. Without this you'd re-stamp
`buttonLastPressedAt` on every loop the button is held, and the timer would never expire.

### Why `INPUT_PULLUP` makes pressed == LOW

```cpp
pinMode(8, INPUT_PULLUP);
```

This enables the ATmega328P's internal ~20–50 kΩ resistor pulling the pin to 5 V. So the
pin reads **HIGH when idle**. Wiring the button between the pin and **GND** means pressing
it drags the pin to **LOW**. That's why "pressed" is the `HIGH -> LOW` edge. It also means
you need no external resistor — the chip provides it.

---

## 6. The loop, frame by frame

```
loop() iteration (runs thousands of times per second):

  read button  --->  falling edge?  --yes-->  stamp millis(), LED = HIGH
       |                  |no
       |                  v
       |            (do nothing)
       v
  elapsed = millis() - buttonLastPressedAt
       |
  elapsed >= time ?  --yes-->  LED = LOW
       |
       v
  digitalWrite(13, ledState)   <-- one write per loop, reflects current state
```

Notice `digitalWrite(13, ledState)` runs **every** loop. The loop never stalls; it just
keeps re-asserting whatever the current `ledState` is. State lives in variables, not in
how long the CPU is stuck.

---

## 7. Embedded relevance

- **Timer0 powers `millis()`.** On the Uno, configuring `millis()` uses Timer0; this is
  also why `delay()`, `micros()`, and `analogWrite()` on pins 5/6 share that timer.
- **`millis()` resolution is 1 ms** but it actually advances in small jumps (every ~1.024
  ms) because of the prescaler math — fine for human-scale timing, not for microsecond
  jobs (use `micros()` there).
- **Cooperative multitasking.** This pattern is how a single-threaded MCU juggles many
  "tasks": each one keeps its own `start` timestamp and checks elapsed time. No RTOS
  needed. (Lesson 22's scheduler generalizes exactly this.)
- **`millis()` does not advance inside an ISR or during `delay()`** in the way you'd hope —
  yet another reason to avoid `delay()`.

---

## Gotchas

| Pitfall | Fix |
|---|---|
| Using `delay(5000)` | Blocks everything — use `millis()` elapsed checks |
| `millis() >= start + interval` | Overflows near 49.7 days — use `millis() - start >= interval` |
| Re-stamping `buttonLastPressedAt` while held | Detect the edge (`prev HIGH && current LOW`), not the level |
| Forgetting `prevPin8state = currentPin8State;` | Edge never resets — press detected only once ever |
| `int` for a timestamp | Timestamps are `unsigned long`; `int` overflows in 32 s |
| Expecting `time = 2000` to mean 5 s | It's milliseconds — 5 s is `5000` |

> Subtle: `buttonLastPressedAt` is declared `unsigned` (i.e. `unsigned int`, 16-bit on AVR,
> max 65535 ms ≈ 65 s) while `millis()` returns `unsigned long` (32-bit). The comparison
> mostly works for short intervals, but for correctness on AVR a timestamp holding a
> `millis()` value should be `unsigned long`.

---

## TL;DR

`delay()` freezes the CPU; `millis()` lets you *check the clock* instead of *waiting on
it*. Record a start time (`buttonLastPressedAt = millis()`), then every loop test
`millis() - buttonLastPressedAt >= time`. The subtract-first form is overflow-safe across
the 49.7-day wrap. Combine it with edge detection so each button press starts a fresh
window — and `loop()` stays responsive the entire time.
