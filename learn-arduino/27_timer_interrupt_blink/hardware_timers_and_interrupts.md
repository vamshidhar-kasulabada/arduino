# Hardware Timers & Interrupts — From Scratch (Challenge #27)

A companion to `27_timer_interrupt_blink.ino`. This assumes you've done Challenge 25/26
(you know bits, masks, `1 << n`, and setting bits in a register), but have **never** set up
a hardware timer or written an interrupt handler. Everything else is defined here.

---

## 0. Glossary (every term used below)

| Term | Plain meaning |
|---|---|
| **clock** | the steady heartbeat that drives the chip; on the Uno it ticks **16,000,000 times/sec** |
| **F_CPU** | a predefined name for that clock frequency = `16000000` (16 MHz) |
| **clock cycle / tick** | one beat of the clock; on the Uno one tick = `1/16,000,000 s` = **62.5 ns** |
| **timer / counter** | a register that **counts up by itself**, one step per (prescaled) clock tick — no code needed |
| **Timer1** | the Uno's **16-bit** timer (counts `0..65535`); we use it here |
| **prescaler** | a divider in front of the timer: feed it clock/1, /8, /64, /256, or /1024 to count slower |
| **TCNT1** | **T**imer/**C**ou**NT**er **1** — the live count, readable/writable |
| **TOP** | the value the timer counts up to before it resets |
| **CTC** | **C**lear **T**imer on **C**ompare match — a mode where TOP is a value you pick |
| **OCR1A** | **O**utput **C**ompare **R**egister 1**A** — the value the timer compares against (our TOP) |
| **TCCR1A/B** | **T**imer/**C**ounter **C**ontrol **R**egisters — pick the mode and prescaler here |
| **TIMSK1** | **T**imer **I**nterrupt **M**a**SK** 1 — switches each timer interrupt on/off |
| **interrupt** | the chip pausing your code to run a special function the instant an event happens |
| **ISR** | **I**nterrupt **S**ervice **R**outine — the function that runs on an interrupt |
| **vector** | the fixed name/address of a specific interrupt (e.g. `TIMER1_COMPA_vect`) |
| **cli() / sei()** | **cl**ear / **se**t the global **i**nterrupt flag = turn all interrupts OFF / ON |
| **volatile** | a keyword telling the compiler "this variable can change behind your back (in an ISR)" |
| **Hz** | hertz = times per second (1 Hz = once per second) |

---

## 1. The problem with `delay()` and even `millis()`

`delay(1000)` blinks an LED, but it **freezes the whole chip** for a second — nothing else
can run. `millis()` is better (Challenge 4): you poll the clock in `loop()` and act when
enough time has passed. But that still means:

- the CPU is **busy looping**, constantly checking the time, and
- the timing is only as accurate as how often your `loop()` gets back around — if `loop()`
  is busy doing something slow, your "every 1000 ms" event **drifts late**.

A **hardware timer** removes both problems. A dedicated counter keeps perfect time *in
hardware*, and taps the CPU on the shoulder (an **interrupt**) only at the exact instant the
event is due. The CPU is free the rest of the time, and the timing never drifts.

(Fun fact: `millis()` itself is built on a hardware timer — Timer0. You're about to do by
hand what the Arduino core does for you.)

---

## 2. The system clock: 16 million ticks a second

The Uno runs at **16 MHz** — the clock ticks `16,000,000` times every second. One tick is
tiny: `1 / 16,000,000 s = 62.5 nanoseconds`. Everything the chip does is paced by this beat.

A timer, left alone, would advance **one count per clock tick**. That's *way* too fast for
human-scale timing:

- An **8-bit** timer (0..255) would overflow every `256 × 62.5 ns ≈ 16 microseconds`.
- A **16-bit** timer (0..65535) would overflow every `65536 × 62.5 ns ≈ 4.1 milliseconds`.

Even the 16-bit timer can't reach a whole second directly. We need to **slow the counting
down** — that's the prescaler.

---

## 3. The prescaler: dividing the clock down

A **prescaler** sits between the clock and the timer and only lets through every Nth tick.
Timer1 offers these divisions, chosen by three bits called **CS12, CS11, CS10** (Clock
Select) in `TCCR1B`:

| CS12 | CS11 | CS10 | Prescaler | Timer ticks per second | One timer tick = |
|---|---|---|---|---|---|
| 0 | 0 | 0 | (stopped) | — | — |
| 0 | 0 | 1 | 1     | 16,000,000 | 62.5 ns |
| 0 | 1 | 0 | 8     | 2,000,000  | 0.5 µs |
| 0 | 1 | 1 | 64    | 250,000    | 4 µs |
| **1** | **0** | **0** | **256** | **62,500** | **16 µs** |
| 1 | 0 | 1 | 1024  | 15,625     | 64 µs |

We'll use **prescaler 256** (set bit `CS12`). At /256 the timer ticks **62,500 times per
second** — slow enough that a 16-bit counter can comfortably measure a full second.

---

## 4. CTC mode: count up to a number you choose, then reset

By default a timer counts `0 → 65535 → 0 → …` (it wraps at its max). **CTC mode** ("Clear
Timer on Compare match") lets you pick your *own* TOP value: the timer counts `0 → OCR1A`,
and the instant `TCNT1 == OCR1A` it **resets to 0** and (if enabled) **fires an interrupt**.
It looks like a sawtooth:

```
count
OCR1A ┤      /|      /|      /|        <- reset + interrupt fires here each time
      ┤     / |     / |     / |
      ┤    /  |    /  |    /  |
    0 ┼───/   |___/   |___/   |___ →  time
        one period   one period
```

CTC mode for Timer1 is selected by setting the **WGM12** bit (Waveform Generation Mode) in
`TCCR1B`. With WGM12 set, **OCR1A becomes TOP**.

So two control bits do all the configuration:
- `WGM12` in `TCCR1B` → CTC mode (TOP = OCR1A)
- `CS12`  in `TCCR1B` → prescaler 256

---

## 5. The frequency formula (where 62499 comes from)

You want the interrupt to fire at a chosen frequency. Build it up:

1. After the prescaler, the timer ticks at `F_CPU / prescaler` ticks per second.
   At /256: `16,000,000 / 256 = 62,500` ticks/sec.
2. To make **one period last `1 / freq` seconds**, the timer must count that many ticks:
   `ticks_per_period = (F_CPU / prescaler) / freq`.
   For 1 Hz: `62,500 / 1 = 62,500` ticks.
3. Because the timer counts **starting at 0**, counting `0..N-1` is `N` ticks. So the
   compare value (TOP) is **one less**:

```
   OCR1A = F_CPU / (prescaler * freq) - 1
```

For **1 Hz, prescaler 256**:

```
   OCR1A = 16,000,000 / (256 * 1) - 1 = 62,500 - 1 = 62,499
```

Check it fits: Timer1 is 16-bit, max **65,535**. `62,499 < 65,535` ✓. (If your number came
out bigger than 65,535, you'd pick a larger prescaler — e.g. 1024 — to bring it back in
range.) Sanity check the time: `62,500 ticks × 16 µs/tick = 1,000,000 µs = 1.000 s` ✓.

---

## 6. The registers, bit by bit

Everything is set by writing a few registers (all named for you by `Arduino.h`):

| Register | Role | What we write |
|---|---|---|
| `TCCR1A` | control A | `0` (we use no PWM output pins, so nothing here) |
| `TCCR1B` | control B | `WGM12` (CTC) + `CS12` (prescaler 256) |
| `TCNT1`  | live count | `0` (start fresh) |
| `OCR1A`  | compare target / TOP | `62499` |
| `TIMSK1` | interrupt enable | `OCIE1A` (fire interrupt on compare-match A) |

`OCIE1A` = **O**utput **C**ompare **I**nterrupt **E**nable, channel **1A**. Setting it says
"when TCNT1 hits OCR1A, raise the Compare-Match-A interrupt." Without this bit the timer
still counts and resets — it just won't interrupt.

The setup, assembled (this is what your TODO 2 builds):

```cpp
cli();                       // pause ALL interrupts while we configure (atomic setup)

TCCR1A = 0;                  // clean slate
TCCR1B = 0;
TCNT1  = 0;                  // reset the live counter

OCR1A  = 62499;              // compare target -> 1 Hz at /256

TCCR1B |= (1 << WGM12);      // CTC mode: TOP = OCR1A
TCCR1B |= (1 << CS12);       // prescaler 256 -> timer starts ticking here
TIMSK1 |= (1 << OCIE1A);     // enable compare-match-A interrupt

sei();                       // re-enable interrupts -> the timer is now live
```

Note these are the **same bit-setting idioms from Challenge 25** (`REG |= (1 << BIT)`) —
just on timer registers. Nothing new about the bit math; only the register names are new.

---

## 7. Interrupts and the ISR

An **interrupt** is the hardware pausing your `loop()` mid-stride to run a special function
the instant an event occurs, then resuming exactly where it left off. You met this idea in
Challenge 17 (`attachInterrupt` on a button). Here the event is "Timer1 hit OCR1A."

You write the handler with the `ISR(...)` macro and the **vector name** for that specific
event. For Timer1 compare-match A it's `TIMER1_COMPA_vect`:

```cpp
ISR(TIMER1_COMPA_vect) {
  PORTB ^= (1 << LED_BIT);   // toggle pin 13 -- that's the whole job
}
```

- The name **must** match the interrupt exactly (`TIMER1_COMPA_vect`); a typo compiles but
  silently never runs.
- You don't call an ISR — the **hardware** calls it, automatically, every time the timer
  reaches OCR1A. With OCR1A=62499 at /256, that's once per second, forever, on its own.
- The compare-match flag is **cleared automatically** when the ISR runs — you don't manage it.

**Two rules for ISRs:**

1. **Keep them tiny and fast.** While an ISR runs, other interrupts wait. Toggle a pin, set
   a flag, bump a counter — then get out. No `delay()`, no `Serial.print()` if avoidable, no
   long loops.
2. **Share variables with `volatile`.** If the ISR and `loop()` both touch a variable,
   declare it `volatile` so the compiler always re-reads it from memory (it can't "see" the
   ISR changing it):

```cpp
volatile uint32_t ticks = 0;       // shared between ISR and loop()
ISR(TIMER1_COMPA_vect) { ticks++; }
```

And if `loop()` reads a **multi-byte** shared value (like that 32-bit `ticks`), wrap the read
in `cli()/sei()` so an interrupt can't change it halfway through the read (a "torn read").
For our simple LED toggle there's no shared variable, so we don't need this yet — but it's
the rule the moment the ISR and `loop()` share data.

### Why `cli()` / `sei()` around setup?

`cli()` turns interrupts **off**, `sei()` turns them back **on**. We wrap the configuration
so the timer can't fire *while we're half-way through setting it up* (which could run the ISR
with a bogus, partially-written config). Configure in peace, then `sei()` to go live. This
"do it all at once, uninterrupted" idea is called an **atomic** operation.

---

## 8. How it all maps to the challenge

```cpp
const uint8_t  LED_BIT   = PB5;     // pin 13
const uint16_t OCR1A_1HZ = 62499;   // 1 Hz at /256

void setup() {
  DDRB |= (1 << LED_BIT);           // pin 13 = output

  cli();
  TCCR1A = 0; TCCR1B = 0; TCNT1 = 0;
  OCR1A  = OCR1A_1HZ;
  TCCR1B |= (1 << WGM12);           // CTC
  TCCR1B |= (1 << CS12);            // /256
  TIMSK1 |= (1 << OCIE1A);          // interrupt on compare-match A
  sei();
}

ISR(TIMER1_COMPA_vect) {
  PORTB ^= (1 << LED_BIT);          // toggle the LED -- runs once per second
}

void loop() {
  // empty -- the timer does everything
}
```

The LED toggles every second with an empty `loop()`. The CPU isn't watching a clock; the
**timer** is, and it only interrupts when the second is genuinely up.

---

## 9. Why this is a big deal

- **Exact & jitter-free:** the interval is set by hardware counting clock ticks, not by how
  fast your code loops. 1.000 s, every time.
- **CPU-free:** `loop()` is completely free for other work; the blink keeps perfect time
  regardless of what else runs. (Try the stretch goal — run a busy counter in `loop()` and
  watch the blink stay dead-on.)
- **It's how the real system works:** `millis()`, `delay()`, `tone()`, servo, and PWM are
  all built on these same timer peripherals. You now know the machinery underneath them.

---

## 10. Gotchas cheat-sheet

| Symptom | Cause |
|---|---|
| Nothing blinks at all | forgot `sei()` — interrupts never turned back on |
| ISR never runs | wrong vector name (must be exactly `TIMER1_COMPA_vect`) |
| ISR never runs | forgot to set `OCIE1A` in `TIMSK1` (interrupt not enabled) |
| Timer never counts | forgot the prescaler bits (`CS12…`) — with all CS bits 0 the clock is OFF |
| Wrong/odd frequency | `OCR1A` math off, or wrong prescaler bit set |
| `OCR1A` "doesn't fit" | value > 65535 — use a bigger prescaler (e.g. 1024) |
| Blinks but timing wrong | used a different mode — make sure `WGM12` is set for CTC |
| `millis()` breaks elsewhere | you reconfigured **Timer0** — leave it alone, use Timer1 |
| Shared variable acts stale | forgot `volatile` on a value shared with the ISR |
| Garbled multi-byte read | read a multi-byte `volatile` without `cli()/sei()` (torn read) |

---

## 11. TL;DR

A **hardware timer** is a counter that ticks by itself off the 16 MHz clock. Slow it with a
**prescaler** (`/256` → 62,500 ticks/s, bit `CS12`), run it in **CTC mode** (bit `WGM12`) so
it counts `0 → OCR1A` then resets, and set **OCR1A** from
`F_CPU / (prescaler × freq) − 1` (= 62499 for 1 Hz). Enable the **compare-match interrupt**
(`OCIE1A` in `TIMSK1`), wrap config in `cli()/sei()`, and write
`ISR(TIMER1_COMPA_vect){ PORTB ^= (1<<PB5); }`. The timer keeps perfect time in hardware and
calls your ISR exactly on schedule — an empty `loop()` and a flawless 1 Hz blink. This is the
machinery `millis()` is built on.
