# UART & Serial Communication (Challenge #14)

A companion to `14_serial_command_controller.ino`. This sketch lets your computer drive an
LED by typing `ON`, `OFF`, `TOGGLE`, or `STATUS`. It works because of **UART** — the serial
link between the Arduino and your PC. This note explains UART and the `Serial` API from the
ground up, tied to the code you wrote.

---

## 1. The core idea: talking one bit at a time

**UART** = *Universal Asynchronous Receiver/Transmitter*. It is a tiny piece of hardware
inside the AVR chip that sends and receives data **one bit at a time** over a single wire
per direction. "Serial" just means "one bit after another" (as opposed to parallel, many
bits at once).

On an Arduino Uno, the UART is wired to two pins and bridged over USB to your computer:

```
   Arduino  TX (pin 1) ----> RX   PC (Serial Monitor)
   Arduino  RX (pin 0) <---- TX   PC
              (USB cable carries both directions)
```

`Serial` is the Arduino object that wraps this UART. Everything in this sketch goes through
it: `Serial.begin`, `Serial.available`, `Serial.readStringUntil`, `Serial.println`.

---

## 2. `Serial.begin(9600)` and baud rate

```cpp
void setup() { Serial.begin(9600); }
```

This turns the UART on and sets the **baud rate** to `9600` — roughly 9600 bits per second.
Baud rate is the agreed *speed* of the link.

UART is **asynchronous**: there is no shared clock wire. Instead, **both sides must agree
on the speed in advance**. If your Serial Monitor is set to 115200 but the sketch says
`9600`, you'll see garbage like `‚Äî‚â†` — the receiver samples the wire at the wrong
moments. This is the #1 beginner serial bug: **mismatched baud**.

```
9600 baud  ->  ~960 bytes/sec  (each byte = 1 start + 8 data + 1 stop = 10 bits)
```

---

## 3. The UART frame

Each byte does not travel naked. The UART wraps it in a **frame** so the receiver knows
where a byte starts and stops:

```
 idle   start     8 data bits (LSB first)        stop   idle
 ----+ +-----+ +--+--+--+--+--+--+--+--+ +-----+ +----
     | |  0  | |b0 b1 b2 b3 b4 b5 b6 b7| |  1  | |
     +-+     +-+                        +-+     +-+
       \start bit          \"8N1" = 8 data, No parity, 1 stop bit (the default)
```

You never write this by hand — the hardware does it. But it explains *why* speed must
match: the receiver uses the falling **start bit** to line up, then samples each following
bit at fixed time intervals derived from the baud rate.

---

## 4. `Serial.available()` — don't block, poll

```cpp
void loop() {
  if (Serial.available() > 0) {
    ...
  }
}
```

Incoming bytes land in a small **receive buffer** (64 bytes on an Uno) as they arrive.
`Serial.available()` returns **how many bytes are waiting** in that buffer right now.

The `if (Serial.available() > 0)` guard means: "only try to read if there's actually
something there." Without it, a read would either block or return junk. Because `loop()`
runs thousands of times per second, this is **polling** — the sketch keeps checking, and
the rest of the time it's free to do other work. (The alternative, an RX interrupt, fires
only when a byte arrives, but is overkill here.)

---

## 5. `readStringUntil('\n')` — framing the command

```cpp
String command = Serial.readStringUntil('\n');
command.trim();
```

The bytes arrive one at a time, so where does a "command" end? By a **delimiter**. When you
press Enter in the Serial Monitor (with line ending set to *Newline*), it appends `'\n'`.

- `readStringUntil('\n')` reads bytes into a `String` until it hits that newline (or times
  out), giving you one whole line.
- `command.trim()` strips leading/trailing whitespace — crucially the `'\r'` that some
  terminals add (*Carriage Return + Newline*, `\r\n`). Without `trim()`, the string would be
  `"ON\r"`, and `command.equals("ON")` would silently fail. **This single line prevents a
  very common "my command does nothing" bug.**

```
typed:  O  N  \r \n
buffer: 'O' 'N' '\r' '\n'
readStringUntil('\n')  ->  "ON\r"
       .trim()         ->  "ON"
```

---

## 6. Dispatching the command

With one clean line in hand, the rest is straightforward comparison:

```cpp
if (command.equals("ON")) {
  led.on();
  Serial.println("LED ON");
} else if (command.equals("OFF")) {
  led.off();
  Serial.println("LED OFF");
} else if (command.equals("TOGGLE")) {
  led.toggle();
} else if (command.equals("STATUS")) {
  if (led.isOn()) Serial.println("LED is ON");
  else            Serial.println("LED is OFF");
} else {
  Serial.println("UNKNOWN COMMAND");
}
```

- `command.equals("ON")` is an **exact, case-sensitive** match (the spec requires this), so
  `on` or `On` would fall through to `UNKNOWN COMMAND`.
- Notice the LED itself is fully reused from challenge 13: `led.on()`, `led.off()`,
  `led.toggle()`, `led.isOn()`. The serial layer never touches `digitalWrite` — it just
  translates text into method calls.
- `Serial.println(...)` sends the reply **back** over the same UART, so you see feedback in
  the Serial Monitor.

```
  PC types "TOGGLE\n"
        |
        v  (UART RX)
  Serial.available() > 0
        |
  readStringUntil('\n') -> "TOGGLE\r" -> trim() -> "TOGGLE"
        |
  command.equals("TOGGLE") ? --> led.toggle()
        |
  (no reply for TOGGLE in this sketch)
```

The full command-flow: **bytes in -> assemble a line -> match -> act on hardware -> reply.**
That is the bones of every serial command interface.

---

## 7. Embedded relevance

- **Runtime control.** UART lets you change behavior *without reflashing*. The same firmware
  obeys whatever you type — invaluable for debugging and field tuning.
- **`Serial.println` is your printf.** A microcontroller has no screen; serial output is how
  you watch what the chip is thinking. It is the embedded developer's primary debug tool.
- **The buffer is small.** The 64-byte RX buffer can **overflow** if data arrives faster than
  `loop()` drains it; extra bytes are silently dropped. Keep `loop()` fast and don't block.
- **`String` on AVR.** Arduino's `String` uses the heap. It's convenient for a learning
  sketch, but in long-running production firmware it can fragment RAM — fixed `char[]` buffers
  are preferred there (see challenge 15's parsing for a taste of working with text).

---

## 8. Gotchas cheat-sheet

| Symptom | Cause | Fix |
|---|---|---|
| Garbage characters in monitor | Baud mismatch | Set monitor to 9600 to match `Serial.begin(9600)` |
| Commands "do nothing" | Trailing `\r` not stripped | Keep `command.trim()`; set monitor line ending to Newline |
| `on` / `On` rejected | `equals()` is case-sensitive | Type exactly `ON`, or use `equalsIgnoreCase()` |
| `loop()` seems stuck waiting | Used a blocking read without `Serial.available()` guard | Poll with `if (Serial.available() > 0)` |
| Lost characters at high speed | RX buffer overflow | Don't block in `loop()`; lower data rate |

- `Serial.available()` = bytes waiting; it does **not** wait for them.
- A delimiter (`'\n'`) is what turns a byte stream into discrete commands.

---

## TL;DR

UART is the chip's built-in hardware for sending data **one bit at a time** over the USB
serial link; `Serial` is the Arduino wrapper. You start it with `Serial.begin(9600)` (both
sides must agree on the **baud rate**), check `Serial.available()` to poll for incoming
bytes without blocking, and use `readStringUntil('\n')` + `trim()` to assemble one clean
command line. A chain of `command.equals(...)` checks then maps text to `Led` method calls,
and `Serial.println` sends feedback back. That bytes -> line -> match -> act -> reply loop is
how a microcontroller is controlled at runtime from a computer.
