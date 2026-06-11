# Registers & Bitwise Operations — From Scratch (Challenge #25)

A companion to `25_port_manipulation.ino`. This assumes you have **never** used binary,
bitwise operators, shifting, or registers. Everything is defined here — read top to
bottom and you won't need to look anything up elsewhere.

---

## 0. Quick glossary (every abbreviation used below)

| Term | Stands for | Plain meaning |
|---|---|---|
| **bit** | binary digit | a single `0` or `1` |
| **byte** | — | a group of **8 bits** |
| **nibble** | — | a group of **4 bits** (half a byte) |
| **MCU** | Microcontroller Unit | the chip on the Arduino (Uno = ATmega328P) |
| **AVR** | — | the family/brand of that 8-bit chip |
| **I/O** | Input / Output | reading inputs, driving outputs |
| **GPIO** | General-Purpose I/O | a pin you can use as input or output |
| **register** | — | a small box of bits *inside the chip* at a fixed address |
| **DDR** | Data Direction Register | sets each pin to input or output |
| **PORT** | (output register) | sets each output pin HIGH or LOW |
| **PIN** | (input register) | reads the level of each pin |
| **PBx** | Port B, bit x | name for one bit, e.g. `PB2` = bit 2 of Port B |
| **LSB** | Least Significant Bit | the **rightmost** bit (value 1) |
| **MSB** | Most Significant Bit | the **leftmost** bit (value 128 in a byte) |
| **mask** | — | a number used to pick *which bits* to change |
| **hex** | hexadecimal | base-16 shorthand for writing bits |

---

## 1. Counting in binary

You already count in **decimal** (base 10): ten digits `0–9`, and each position is worth
10× more as you go left:

```
 357  =  3×100  +  5×10  +  7×1
```

**Binary** (base 2) uses only **two** digits, `0` and `1`, and each position is worth 2×
more as you go left. The position values are `1, 2, 4, 8, 16, 32, 64, 128, ...`:

```
position value:  128  64  32  16   8   4   2   1
binary digit:      0   0   0   0   0   1   0   1
```

To read a binary number, **add up the position values wherever there is a 1**:

```
0b00000101  =  4 + 1  =  5
0b00000111  =  4 + 2 + 1  =  7
0b11111111  =  128+64+32+16+8+4+2+1  =  255
```

(`0b` is just how you write "this is binary" in C++ — see §4.)

Why binary? Because hardware is physically two-state: a wire is either at ~5 volts
(**HIGH**, `1`) or ~0 volts (**LOW**, `0`). A `1` bit can literally mean "this pin is on."

---

## 2. Bits, bytes, and bit numbering

- A **bit** is one `0` or `1`.
- A **byte** is **8 bits** in a row.
- We **number the bits from the right, starting at 0**:

```
bit number:   7   6   5   4   3   2   1   0
value:      128  64  32  16   8   4   2   1
            └ MSB ┘                   └ LSB ┘
```

- **Bit 0** (rightmost, value 1) is the **LSB** — Least Significant Bit.
- **Bit 7** (leftmost, value 128) is the **MSB** — Most Significant Bit.

A byte holds **0 to 255** — that's 2⁸ = 256 different patterns (`00000000` to `11111111`).
(This is exactly why `analogWrite` goes 0–255: it's one byte.)

---

## 3. Hexadecimal (a shorthand for bits)

Writing `0b11111111` is tedious, so we often use **hex** (base 16). Hex digits are
`0–9` then `A B C D E F` (A=10 … F=15). The trick: **one hex digit = exactly 4 bits**
(one nibble), so **two hex digits = one byte**.

| Bits | Hex | Decimal | | Bits | Hex | Decimal |
|---|---|---|---|---|---|---|
| 0000 | 0 | 0 | | 1000 | 8 | 8 |
| 0001 | 1 | 1 | | 1001 | 9 | 9 |
| 0010 | 2 | 2 | | 1010 | A | 10 |
| 0011 | 3 | 3 | | 1011 | B | 11 |
| 0100 | 4 | 4 | | 1100 | C | 12 |
| 0101 | 5 | 5 | | 1101 | D | 13 |
| 0110 | 6 | 6 | | 1110 | E | 14 |
| 0111 | 7 | 7 | | 1111 | F | 15 |

So `0xFF` = `1111 1111` = 255, and `0x07` = `0000 0111` = 7. Hex and binary describe the
**same number** — hex is just more compact.

---

## 4. Writing numbers in C++

These are three ways to write the **same value**, `7`:

```cpp
int a = 7;          // decimal
int b = 0b00000111; // binary  (0b prefix)
int c = 0x07;       // hex     (0x prefix)
// a == b == c, all equal 7
```

Use whichever is clearest. For bit work, binary/hex make the pattern obvious.

---

## 5. What is a "register"?

The MCU (the chip) has many tiny 8-bit storage boxes called **registers**, each at a fixed
address in memory. Some registers are special: their bits are **wired directly to the
physical pins**. Writing a `1` into such a bit makes the matching pin go HIGH; writing `0`
makes it go LOW. That is literally how software moves hardware. (This idea — hardware
controlled by reading/writing memory addresses — is called **memory-mapped I/O**.)

You use these registers by name (`PORTB`, etc.) as if they were normal variables. The
names are pre-defined for you by `Arduino.h`.

---

## 6. The three Port B registers on the Uno

The Uno's pins are split into groups called **ports** (B, C, D). **Port B** contains
digital pins **8 to 13**:

```
Port B bit:   7    6    5    4    3    2    1    0
Uno pin:    (xtal)(xtal) 13   12   11   10    9    8
name:        --   --    PB5  PB4  PB3  PB2  PB1  PB0
```

`PBx` just means "Port B, bit x" — so `PB0` is the number `0`, `PB2` is `2`, etc. They're
names for bit positions. (Bits 6–7 are wired to the clock crystal, not usable as pins.)

Port B has **three** registers, each 8 bits wide, one bit per pin:

| Register | Full name | What each bit does |
|---|---|---|
| `DDRB`  | **D**ata **D**irection **R**egister B | `1` = make this pin an **output**, `0` = **input** |
| `PORTB` | the output register | for an output pin: `1` = drive **HIGH**, `0` = **LOW** |
| `PINB`  | the input register | **read** this to see each pin's level |

So controlling a pin is a two-step idea: set its **direction** bit in `DDRB`, then set its
**value** bit in `PORTB`.

---

## 7. The bitwise operators (the heart of it)

"**Bitwise**" means the operator works on **each bit position independently**. There are
four, plus shifting (§8). For each, line the two numbers up and apply the rule per column.

### AND — `&` — "1 only if BOTH are 1"

```
  A: 1 1 0 0
  B: 1 0 1 0
A&B: 1 0 0 0      (1 only where both have 1)
```
Use: **force bits to 0** (a `0` in B zeroes that column; a `1` keeps it).

### OR — `|` — "1 if EITHER is 1"

```
  A: 1 1 0 0
  B: 1 0 1 0
A|B: 1 1 1 0      (1 wherever either has 1)
```
Use: **force bits to 1** (a `1` in B sets that column; a `0` keeps it).

### XOR — `^` — "1 if they DIFFER"

```
  A: 1 1 0 0
  B: 1 0 1 0
A^B: 0 1 1 0      (1 only where they're different)
```
Use: **flip/toggle bits** (a `1` in B flips that column; a `0` keeps it).

### NOT — `~` — "flip every bit"

```
 ~(0000 0101) = 1111 1010      (every 0→1 and 1→0)
```
Use: build the "opposite" of a mask.

> Memory hook: **AND clears, OR sets, XOR toggles, NOT inverts.**

---

## 8. Shifting — `<<` and `>>`

Shifting slides all the bits left or right.

- **`<<` (left shift):** move bits left, fill `0` on the right.
- **`>>` (right shift):** move bits right.

```
   0000 0001  <<  0   =  0000 0001   (value 1)
   0000 0001  <<  1   =  0000 0010   (value 2)
   0000 0001  <<  2   =  0000 0100   (value 4)
   0000 0001  <<  3   =  0000 1000   (value 8)
```

The pattern you'll use constantly: **`1 << n` makes a number with a single `1` at
position `n`** (and 0s everywhere else). That's how you target one specific pin:

| Expression | Bits | Targets |
|---|---|---|
| `1 << PB0` (=`1<<0`) | `0000 0001` | pin 8 |
| `1 << PB1` (=`1<<1`) | `0000 0010` | pin 9 |
| `1 << PB2` (=`1<<2`) | `0000 0100` | pin 10 |

To **read** one bit, shift it down to position 0 and AND with 1:
`(PINB >> PB2) & 1` → `0` or `1`.

---

## 9. Masks — picking which bits to touch

A **mask** is just a number whose `1` bits mark "these are the bits I care about." You
build one by OR-ing single-bit values together:

```cpp
const uint8_t LED_MASK = (1 << PB0) | (1 << PB1) | (1 << PB2);
//   (1<<PB0) = 0000 0001
//   (1<<PB1) = 0000 0010
//   (1<<PB2) = 0000 0100
//   OR'd     = 0000 0111   = 0x07 = 7   -> "pins 8, 9, 10"
```

(`uint8_t` = an unsigned 8-bit integer, i.e. exactly one byte, 0–255 — the natural type
for an 8-bit register.)

### A mask selects *which* bits; the operator decides *what happens*

Think of a mask as a **stencil**: its `1` bits are the holes you cut, its `0` bits cover
everything else. The mask says *where*; the operator paired with it says *what*:

| Operation | Effect on the **selected** (`1`) bits | The other (`0`) bits |
|---|---|---|
| `REG & MASK`  | **isolate** them (keep/read just those) | cleared to 0 |
| `REG & ~MASK` | **clear / erase** them | kept |
| `REG \| MASK` | **set** them to 1 | kept |
| `REG ^ MASK`  | **toggle** them | kept |

So `LED_MASK` always names the same three pins (8, 9, 10); whether they get set, cleared,
or toggled is decided by the operator, not the mask.

### Why `(1 << PB0) | (1 << PB1) | (1 << PB2)` and not just `0b00000111`?

They produce the **identical byte** — `0b00000111` is perfectly correct. The shift form is
a **readability** choice, not a value difference (the compiler folds it to `7` at build
time, so there's zero runtime cost either way):

- `(1 << PB2)` reads as *"the bit for pin PB2"* — a pin name.
- `0b00000111` makes you **count zeros** to know which pins — easy to miscount across 8 bits.

The gap shows the moment pins aren't a tidy 0,1,2. For the built-in LED on pin 13 = `PB5`:

```cpp
(1 << PB5)     // "the bit for PB5" — obvious
0b00100000     // ...you had to count to bit 5 to write that, and to read it
```

Datasheets name bits by position (`PB5`, `TXEN`, `WGM01`…), so `1 << NAME` lines up with
how the docs talk. For contiguous bits like ours either is fine — just be consistent.

---

## 10. The four idioms (memorize these four lines)

For a single bit `n` in any register `REG`:

```cpp
REG |=  (1 << n);        // SET    bit n to 1   (other bits untouched)
REG &= ~(1 << n);        // CLEAR  bit n to 0   (other bits untouched)
REG ^=  (1 << n);        // TOGGLE bit n        (flip it)
bool on = (REG >> n) & 1;// READ   bit n        (get 0 or 1)
```

`x |= y` is shorthand for `x = x | y` (same for `&=`, `^=`).

**Why CLEAR uses `&= ~(1<<n)`:** `(1<<n)` has a 1 only at n; `~` flips it to **all 1s
except n**; AND-ing keeps every bit but forces n to 0:

```
~(1<<2) = 1111 1011
PORTB:    1010 1110
   &  ->  1010 1010     (bit 2 forced to 0, the rest survive)
```

---

## 11. Read-modify-write (change several bits, keep the rest)

To put a value onto bits 0–2 **without** disturbing pins 11–13, you must NOT write the
whole byte:

```cpp
PORTB = counter;   // WRONG — overwrites all 8 bits, stomps pins 11/12/13
```

Instead: **clear your bits, then OR in the new value**, masking both sides. This pattern
(read the register, modify only some bits, write it back) is called **read-modify-write**:

```cpp
PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);
//        └ keep the others ┘   └ set our 3 bits ┘
```

Worked example, `counter = 6` (`110`), other pins currently on as `x`:

```
PORTB now:   x x x x x 1 0 1
~LED_MASK:   1 1 1 1 1 0 0 0
&        ->  x x x x x 0 0 0   (our 3 bits cleared, rest kept)
counter=6:   0 0 0 0 0 1 1 0
& LED_MASK:  0 0 0 0 0 1 1 0
|        ->  x x x x x 1 1 0   (others intact, LEDs now show 6)
```

### Deriving the formula yourself (don't memorize — rebuild it)

State the goal in plain English. It has **two requirements**:

> "Set bits 0–2 to `counter`, **and** leave bits 3–7 exactly as they were."

You can't satisfy both in one assignment, so build each as its own byte, then merge.

**Analogy:** to change the last 3 digits of `12345` to `678`, you (1) blank the spot →
`12000`, then (2) fill it in → `12678`. Registers work the same, where **erase = AND** and
**write = OR**.

1. **Keep the others, blank my spot** — to blank specific bits, AND with the *inverse*
   mask → `PORTB & ~LED_MASK` (`AND 1` keeps a bit, `AND 0` forces it to 0).
2. **My value, only in my spot** — to keep only my bits of the value → `counter & LED_MASK`.
3. **Merge with OR** — each half has `0` where the other holds real data (complementary
   holes), so OR fills each hole with the other's bits.

**The reusable recipe** (memorize this, not the specific line):

```cpp
REG = (REG & ~MASK) | (VALUE & MASK);
//     └ clear my spot ┘  └ stamp my value ┘
```

Three questions regenerate it every time:
1. **Which bits do I own?** → `MASK`
2. **Keep everyone else?** → `REG & ~MASK`
3. **Drop in my value?** → `| (VALUE & MASK)`

### Can I drop the `& MASK` on the value?

`(REG & ~MASK) | VALUE` gives the same result **only if `VALUE` can never have bits set
outside the mask.** For our `counter` (always 0–7) it's identical — there are no high bits
to strip. But if `counter` were ever `13 = 0b00001101`, bit 3 would **spill onto pin 11**:

```
(PORTB & ~LED_MASK) | counter         counter = 0000 1101
  →  ...0 0 0 0 | 0000 1101 = 0000 1101   ← pin 11 (PB3) lit by accident 💥
```

The `& MASK` is a safety belt that costs nothing (folded at compile time) and guarantees
no spill no matter what `VALUE` becomes. Keep it.

### What breaks if you skip a half (using *your* counter)

| You wrote | What breaks |
|---|---|
| `PORTB \| (counter & MASK)` (no clear) | OR only turns bits **on**; when counter goes 7→0 the LEDs **stay on** — broken counter |
| `(PORTB & ~MASK) \| counter` (no value-mask) | counter's high bits **spill** onto pins 11–13 the moment it exceeds 7 |

Both halves exist for a concrete reason, not ceremony.

### The single-bit idioms (§10) are just shortcuts of this

- `REG |= (1<<n)` → the value is all-1s in that one bit and you only ever *set*, so you can
  skip the clear half.
- `REG &= ~(1<<n)` → you only *clear*, so there's no value-stamp half.

The full `(REG & ~MASK) | (VALUE & MASK)` is the **general form**, for writing an arbitrary
multi-bit value (like your counter).

---

## 12. What `digitalWrite` / `pinMode` really do

Everything you've written so far was these idioms in disguise:

- `pinMode(8, OUTPUT)` ≈ `DDRB |= (1 << PB0);`
- `digitalWrite(8, HIGH)` ≈ `PORTB |= (1 << PB0);`
- `digitalWrite(8, LOW)`  ≈ `PORTB &= ~(1 << PB0);`
- `digitalRead(8)`        ≈ `(PINB >> PB0) & 1;`

The Arduino versions also do safety lookups (which port? disable PWM?) every call — handy
but slower. Direct register writes are one or two CPU instructions, which is why they're
used for fast/precise hardware work and for flipping several pins on one port at once.

---

## 13. How this maps to the challenge

```cpp
const uint8_t LED_MASK = (1<<PB0)|(1<<PB1)|(1<<PB2);  // pins 8,9,10

// setup: make those three pins outputs in ONE write
DDRB |= LED_MASK;

// loop, every 500ms: show counter (0..7) on the low 3 bits, keep the rest
PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);
```

`counter` counts 0→7 and its three bits light the LEDs as a binary number. That's the
whole challenge.

---

## 14. Gotchas cheat-sheet

| Symptom | Cause |
|---|---|
| Pin 13 / others misbehave | wrote the whole byte (`PORTB = x`) instead of masking |
| Pin never lights | forgot to set its `DDRB` (direction) bit to output |
| Everything else turns off | used `=` where you meant `\|=` (`=` overwrites all 8 bits) |
| Wrong LED changes | mixed up the bit number — pin 10 is `PB2`, not `PB10` |
| `1 << 20` gives garbage | plain `1` is a 16-bit `int`; for big shifts use `1UL << n` |
| `&` / `&&` confusion | `&` is bitwise (per-bit); `&&` is logical (true/false). Use `&` here. |

---

## 15. Building the intuition (derive these, don't memorize them)

The goal isn't to memorize tricks like `(counter + 1) & 7` — it's to *see why* they work
so you can rebuild any of them at the keyboard. A few mental habits get you there.

### Habit 1 — see **columns**, not a number

A byte is 8 independent columns with fixed values:

```
128  64  32  16   8   4   2   1
```

Every bitwise operator acts on **each column on its own**. The moment you picture columns
instead of a number, `& | ^ ~` stop being arithmetic and become simple per-column rules
("keep this column", "force it on", "flip it"). That single shift removes most of the magic.

### Habit 2 — read every constant as its **bit pattern**

When you meet a "magic number", immediately ask *"what does that look like in binary?"* —
the pattern tells you the constant's **job**:

| Constant | Binary | Its job |
|---|---|---|
| `7`    | `0000 0111` | the low 3 bits / "mod 8" |
| `0x0F` | `0000 1111` | the low nibble (4 bits) |
| `0xFF` | `1111 1111` | the whole byte |
| `0x80` | `1000 0000` | just the top bit (bit 7) |
| `1<<n` | one `1` at position n | "target pin n" |

`& 7` isn't a trick once you see `7` as `0b111` = "keep the bottom three bits."

### Habit 3 — learn the three **power-of-2 identities** (they explain most "tricks")

```
x << n   ==  x * 2ⁿ        (shift left  = multiply by a power of 2)
x >> n   ==  x / 2ⁿ        (shift right = divide   by a power of 2)
x & (2ⁿ − 1)  ==  x mod 2ⁿ (mask of n low 1s = remainder mod 2ⁿ)
```

> The third is the big one. `2ⁿ − 1` is exactly "n ones" (`8 − 1 = 7 = 0b111`), so AND-ing
> with it keeps the low n bits — which **is** the remainder after dividing by `2ⁿ`.

### Worked example — why `(counter + 1) & 7` wraps 7 → 0

`& 7` forces the result into `0..7` by deleting everything above bit 2. At the rollover,
`counter` is `7`, and `+1` carries up into bit 3 — a bit the mask throws away:

```
  8 = 0000 1000      ← the +1 carried into bit 3
  7 = 0000 0111      ← mask (keep low 3 bits)
  & ───────────
      0000 0000  = 0     ← bit 3 isn't in the mask → discarded → wraps to 0
```

So `(counter + 1) & 7` is just `(counter + 1) mod 8`. The wrap is **free** because the range
is a power of 2 and the overflow lands in a discarded bit. (If your range were *not* a power
of 2 — say a die, `1..6` — masking can't do it; you'd need `% 6` or an explicit `if`.)

### Same trick, stepping by 2 — it still wraps, but watch what it skips

`(counter + 2) & 7` = `(counter + 2) mod 8` still stays in range, but the **sequence** is:

```
0 → 2 → 4 → 6 → (8&7)=0 → 2 → …      only the EVEN values; 1,3,5,7 never appear
```

Because the step (2) shares a factor with 8, you only visit half the slots. A step that's
**coprime** to 8 (`1, 3, 5, 7`) visits *all* eight before repeating — e.g. step 3 gives
`0,3,6,1,4,7,2,5,0`. Lesson: `& 7` answers *"stay in range and wrap"*, **not** *"hit every
value"* — knowing which question a trick answers is half the skill.

### Habit 4 — go **goal → code**, and verify by hand

- Memorize **intents**, then rebuild the operator:
  "select bits" → `& mask` · "force on" → `| mask` · "flip" → `^ mask` ·
  "target bit n" → `1 << n` · "stay in a 2ⁿ range / wrap" → `& (2ⁿ − 1)`.
- When unsure, **write the columns out** on a tiny example and apply the op by hand (like
  the traces above). Do it enough times and it becomes reflex — that *is* the intuition.

---

## 16. TL;DR

A number is a row of bits; **bit 0 is the rightmost (value 1)**. A **register** is an 8-bit
box in the chip wired to pins: `DDRB` sets input/output, `PORTB` sets HIGH/LOW, `PINB`
reads. **`1 << n`** points at bit `n`. Then: **`|=` sets, `&= ~` clears, `^=` toggles,
`(REG>>n)&1` reads.** To change several bits but keep the rest, **read-modify-write**:
`PORTB = (PORTB & ~mask) | (value & mask)`. Every `digitalWrite` you've ever called is
exactly this, wrapped up for convenience.
