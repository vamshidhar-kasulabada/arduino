# String Parsing & Command Processing (Challenge #15)

A companion to `15_multi_led_controller.ino`. Challenge 14 controlled **one** LED with whole-
word commands. Here a command carries an **argument** — `ON 1`, `OFF 2`, `TOGGLE 3` — so you
must *parse* the text to pull out the number, validate it, then act on the right LED. This
note explains String parsing and command processing from the ground up, tied to your code.

---

## 1. The core idea: a command now has parts

In challenge 14 a command was a single token (`ON`). Now it has **two parts**: a verb and an
LED number.

```
"ON 1"
 ^^ ^
 |  └── argument: which LED (1-based)
 └───── verb: what to do
```

Parsing means splitting that line into its meaningful pieces. Your sketch does it with two
helpers (`getLedNumber`) and Arduino `String` methods (`startsWith`, `lastIndexOf`,
`substring`, `toInt`).

---

## 2. Reading one line (same as before)

```cpp
String command = Serial.readStringUntil('\n');
command.trim();
```

Just like challenge 14: read up to the newline into a `String`, then `trim()` off the
stray `'\r'` / spaces. After this, `command` is a clean line such as `"ON 1"` or `"STATUS"`.

---

## 3. Extracting the number: `getLedNumber`

```cpp
int getLedNumber(String s) {
  int numIndex = s.lastIndexOf(" ");   // position of the LAST space
  int ledNum   = s.substring(numIndex).toInt();
  return ledNum;
}
```

Walk through it with `"ON 1"`:

```
 index:  0 1 2 3
 char:   O N _ 1        ( _ = space )

 lastIndexOf(" ")  -> 2          (the space sits at index 2)
 substring(2)      -> " 1"       (everything from index 2 to the end)
 toInt()           -> 1          (parse the leading number; ignores the space)
```

Three `String` methods doing one job each:

- **`lastIndexOf(" ")`** finds the position of the last space, so it works even for verbs
  with no space (`STATUS` returns `-1`).
- **`substring(numIndex)`** slices from that position to the end of the string — the tail
  that holds the number.
- **`toInt()`** converts the text `" 1"` into the integer `1`. If the text has no number,
  `toInt()` returns `0` (handy: `0` is an invalid LED, so it gets rejected naturally).

> Note the off-by-feel detail: `substring(2)` includes the space, but `toInt()` skips
> leading whitespace, so `" 1"` still parses to `1`. For `STATUS`, `lastIndexOf` returns
> `-1` and `substring(-1)` gives the whole/empty tail whose `toInt()` is `0`.

---

## 4. Validating the index: `isNumValid`

Never trust input. A user can type `ON 4` or `ON 99` for LEDs that don't exist. Before you
index the array, you check the number is in range:

```cpp
Led leds[] = {Led(8), Led(9), Led(10)};
int size = sizeof(leds) / sizeof(leds[0]);   // = 3, computed, not hard-coded

bool isNumValid(int n) { return n >= 1 && n <= size; }
```

- `size` is derived with the classic **array-length idiom**: total bytes / bytes-per-element.
  Add a fourth LED to the array and `size` updates itself — nothing else changes.
- `isNumValid` accepts `1..size` (1-based, matching how a human counts LEDs).

This guards the array access below. Indexing an array out of bounds on an AVR doesn't crash
politely — it reads/writes **random RAM**, which is why validation matters.

---

## 5. Dispatching: verb + validated index -> action

```cpp
int ledNum = getLedNumber(command);
if (isNumValid(ledNum)) {
  if (command.startsWith("ON")) {
    leds[ledNum - 1].on();
  } else if (command.startsWith("OFF")) {
    leds[ledNum - 1].off();
  } else if (command.startsWith("TOGGLE")) {
    leds[ledNum - 1].toggle();
  }
} else if (command.equals("STATUS")) {
  ...
} else {
  Serial.println("INVALID LED");
}
```

Two important parsing choices here:

- **`startsWith("ON")`** matches the *verb* at the front of the line, ignoring the trailing
  number. That's why `"ON 1"` matches the `ON` branch even though the whole string isn't `"ON"`.
- **`leds[ledNum - 1]`** is the **1-based -> 0-based** conversion. The user says LED `1`; the
  array stores it at index `0`. Forgetting this `- 1` is the classic off-by-one bug — it would
  light the wrong LED and let `ledNum == size` walk off the end.

The full command-processing flow:

```
  "TOGGLE 3\n"
        |
  readStringUntil('\n') -> "TOGGLE 3\r" -> trim() -> "TOGGLE 3"
        |
  getLedNumber()  --(lastIndexOf/substring/toInt)-->  3
        |
  isNumValid(3)?  --no-->  "STATUS"? --no--> "INVALID LED"
        | yes
  startsWith("TOGGLE")?  --yes-->  leds[3 - 1].toggle()  // index 2
```

---

## 6. `STATUS`: looping the array

```cpp
} else if (command.equals("STATUS")) {
  for (int i = 1; i <= size; i++) {
    Serial.print("LED");
    Serial.print(i);
    Serial.print(" : ");
    if (leds[i - 1].isOn()) Serial.println("ON");
    else                    Serial.println("OFF");
  }
}
```

Because the LEDs live in an **array**, reporting all of them is one tidy loop instead of
three copy-pasted blocks. Again note `leds[i - 1]` doing the 1-based-to-0-based shift, and
that the printout is built piece by piece with `print` / `println`.

> The commented-out line above it,
> `// Serial.println(" : " + leds[i - 1].isOn() ? "ON" : "OFF");`, is left in as a reminder
> of a real trap: `+` binds tighter than `?:`, so that expression does **not** do what it
> looks like. Splitting it into explicit `if/else` (as the working code does) is clearer and
> correct.

---

## 7. Why an array beats three named variables

The spec forbids `redLed / yellowLed / greenLed` and demands an array — for good reason:

```cpp
Led leds[] = {Led(8), Led(9), Led(10)};   // scales by editing ONE line
```

- **Component scaling.** One LED or ten, the parsing, validation, and dispatch code is
  *identical*. You moved from "one LED" to "N LEDs" with the same structure — the learning
  goal of this challenge.
- **Index by data.** `leds[ledNum - 1]` turns a *runtime number* into the right object. With
  named variables you'd need a `switch` on every command.

---

## 8. Embedded relevance

- **Parse-then-validate-then-act** is the universal shape of a command interface, from this
  toy up to AT-command modems and GRBL motion controllers.
- **Input is hostile.** `isNumValid` and the `else` -> `"INVALID LED"` path exist because
  out-of-range array access on an MCU corrupts memory silently. Bounds-checking is not
  optional in firmware.
- **`String` cost.** Arduino `String` is heap-backed; passing `String s` *by value* into
  `getLedNumber` copies it. Fine for a lesson, but production firmware favors fixed `char[]`
  buffers with `strtok` / `atoi` to keep RAM deterministic.

---

## 9. Gotchas cheat-sheet

| Symptom | Cause | Fix |
|---|---|---|
| Wrong LED lights up | Forgot 1-based -> 0-based shift | Index with `leds[ledNum - 1]` |
| `ON 1` falls through to error | Used `equals("ON")` instead of `startsWith("ON")` | Match the verb prefix, not the whole line |
| Out-of-range number crashes/garbles | No bounds check before indexing | Guard with `isNumValid(ledNum)` |
| `STATUS` printed as "INVALID LED" | `STATUS` has no number, so `getLedNumber` -> 0 (invalid) | Handle `STATUS` in the `else` branch (as the code does) |
| `" : " + bool ? ...` prints wrong thing | `+` binds tighter than `?:` | Use explicit `if/else` for the ON/OFF text |

- `lastIndexOf(" ")` returns `-1` when there is no space; `toInt()` of non-numbers is `0`.
- `size = sizeof(leds)/sizeof(leds[0])` so the array length is never hard-coded.

---

## TL;DR

When a command carries an argument (`ON 1`), you must **parse** it: `getLedNumber` uses
`lastIndexOf(" ")`, `substring`, and `toInt` to pull the number out, `isNumValid` checks it
against the computed array `size`, and `startsWith` matches the verb so the right
`leds[ledNum - 1]` method fires. Storing the LEDs in an **array** plus a 1-based -> 0-based
index lets the exact same parse -> validate -> act pipeline scale from one LED to N without
rewriting the logic.
