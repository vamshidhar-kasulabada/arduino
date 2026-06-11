# Arduino Development Setup

This document describes how to build, upload and monitor Arduino sketches in this repository.

> **On Windows WSL2?** USB devices aren't visible to WSL by default. See
> [`wsl-setup.md`](./wsl-setup.md) for the USB/IP forwarding steps; the build/upload
> commands here apply once the board shows up as `/dev/ttyUSB0`.

---

## Prerequisites

Install:

* Arduino CLI
* Git

Verify installation:

```bash
arduino-cli version
git --version
```

---

## Clone Repository

```bash
git clone <repository-url>
cd arduino
```

---

## Discover Connected Board

List connected boards:

```bash
arduino-cli board list
```

Example:

```text
Port                Protocol  Type              Board Name
/dev/cu.usbserial   serial    Arduino Uno       arduino:avr:uno
```

---

## Compile Sketch

Move into the sketch directory:

```bash
cd learn-arduino/01_blink_led
```

Compile:

```bash
arduino-cli compile --fqbn arduino:avr:uno .
```

---

## Upload Sketch

Upload to the connected board:

```bash
arduino-cli upload -p /dev/cu.usbserial-120 --fqbn arduino:avr:uno .
```

Replace the port with the one shown by:

```bash
arduino-cli board list
```

---

## Serial Monitor

Open serial monitor:

```bash
arduino-cli monitor -p /dev/cu.usbserial-120 -c baudrate=9600
```

Exit:

```text
Ctrl + C
```

---

## Personal Shell Aliases

The following aliases are used frequently while working on sketches.

### Compile

```bash
ac
```

Equivalent to:

```bash
arduino-cli compile --fqbn arduino:avr:uno .
```

---

### Upload

```bash
au
```

Equivalent to:

```bash
arduino-cli upload -p /dev/cu.usbserial-120 --fqbn arduino:avr:uno .
```

---

### Monitor

```bash
am
```

Equivalent to:

```bash
arduino-cli monitor -p /dev/cu.usbserial-120 -c baudrate=9600
```

---

## Typical Workflow

Compile:

```bash
ac
```

Compile and upload:

```bash
ac && au
```

Compile, upload and open monitor:

```bash
ac && au && am
```

This is the most commonly used workflow.

---

## Project Structure

```text
arduino/
└── learn-arduino/
    ├── 01_blink_led/
    ├── 02_button_input/
    ├── 03_led_toggle/
    ├── ...
    └── lib/
```

Each sketch is self-contained and can be compiled from its own directory.

Example:

```bash
cd learn-arduino/17_interrupt_button

ac && au && am
```

---

## Troubleshooting

### Board Not Detected

Check:

```bash
arduino-cli board list
```

Reconnect USB cable if no board appears.

---

### Upload Failed

Check:

* Correct port
* Correct board type
* Serial monitor is closed

---

### Serial Monitor Shows Nothing

Verify:

```cpp
Serial.begin(9600);
```

and monitor baud rate:

```text
9600
```

