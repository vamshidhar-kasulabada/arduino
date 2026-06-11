# PWM & Analog Output (Challenge #24)

A companion to `24_pwm_led_fade.ino`. How a digital pin that only knows HIGH/LOW can make
an LED look half-bright — and why that same trick runs motors, servos, and tone.

---

## 1. The problem

A digital pin has exactly two states:

```cpp
digitalWrite(pin, HIGH);  // 5V
digitalWrite(pin, LOW);   // 0V
```

There is no "2.5V" instruction. So how do you get *half* brightness? You don't lower the
voltage — you **blink faster than the eye can see** and control *how much* of the time
it's on.

---

## 2. PWM = Pulse Width Modulation

PWM switches the pin on and off hundreds/thousands of times per second. The knob you turn
is the **duty cycle** — the fraction of each cycle the pin is HIGH:

```
 0% duty   ________________________   (always LOW)   -> off
25% duty   ▔▔___▔▔___▔▔___▔▔___       -> dim
50% duty   ▔▔▔▔____▔▔▔▔____           -> half bright
75% duty   ▔▔▔▔▔▔__▔▔▔▔▔▔__           -> bright
100% duty  ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔   (always HIGH) -> full
```

Because the switching is far faster than your eye (or a motor, or a speaker) can react,
what you perceive is the **average** — i.e. the duty cycle. The LED isn't dimmer; it's
*off part of the time*, and your eye averages it out.

---

## 3. `analogWrite(pin, value)`

On Arduino you set the duty cycle with one call:

```cpp
analogWrite(pin, value);   // value 0..255  (8-bit)
//   0   -> 0%   duty  -> off
//   64  -> 25%  duty
//   127 -> ~50% duty  -> half bright
//   191 -> 75%  duty
//   255 -> 100% duty  -> full on
```

`value / 255` is the duty cycle. The name `analogWrite` is a bit of a lie — the output is
still digital pulses, *not* a true analog voltage. It just *averages* to one.

---

## 4. Only PWM (`~`) pins work

`analogWrite` needs hardware timers, so it works only on the pins marked `~` on the board.
On the **Uno**: pins **3, 5, 6, 9, 10, 11**.

That's why this challenge uses **pin 9** and not pin 8 — pin 8 has no PWM hardware, so
`analogWrite(8, 127)` would just behave like on/off, not a fade.

```cpp
const uint8_t LED_PIN = 9;   // ~ pin. Pin 8 would NOT fade.
```

---

## 5. Frequency: fast enough to look smooth

The Uno's PWM runs at roughly **490 Hz** (≈980 Hz on pins 5 & 6). That's ~490 on/off
cycles per second — way past the ~60 Hz where your eye stops seeing flicker, so a steady
duty cycle looks like a steady brightness. You normally don't change the frequency; you
only change the *duty cycle* via `analogWrite`.

---

## 6. Why a linear fade can look uneven (gamma)

Your eye's brightness perception is **non-linear** — it's very sensitive at the dim end
and less so at the bright end. So stepping `brightness` in equal `+1` increments (a
*linear* ramp) often looks like it "jumps" early then crawls near full. That's not a bug.

If you want a fade that looks even, map the value through a curve (gamma correction),
e.g. `out = (in*in) / 255`. Nice as a later refinement — the linear version is the right
place to start.

---

## 7. Same trick, everywhere

Once you have "average voltage via duty cycle," a lot of hardware falls out of it:

- **LED brightness** — what you're doing now.
- **DC motor speed** — higher duty = faster (via a driver, not straight off the pin).
- **Servo angle** — a specific pulse-width scheme (`Servo` library wraps it).
- **Tone / buzzers** — `tone()` uses square waves at audible frequencies.

PWM is one of the highest-leverage embedded concepts for that reason.

---

## 8. Walk through the fade

The non-blocking shape is the same `millis()` pattern from your timer challenges, but
instead of toggling you ramp a 0–255 value:

```cpp
if (millis() - timer >= STEP_MS) {     // time to take one step?
  timer += STEP_MS;
  brightness += direction;             // +1 up, -1 down
  if (brightness == 0 || brightness == 255)
    direction = -direction;            // bounce at the ends
  analogWrite(LED_PIN, brightness);    // apply the new duty cycle
}
```

`STEP_MS` controls fade *speed* (smaller = faster). `direction` makes it bounce between
off and full instead of jumping back to 0.

---

## 9. Gotchas cheat-sheet

| Symptom | Cause |
|---|---|
| LED only on/off, won't fade | pin isn't a `~` PWM pin (e.g. 8), or you used `digitalWrite` |
| `value` out of range | `analogWrite` expects 0–255; clamp your `brightness` |
| Flicker | not from PWM (490 Hz) — usually a slow/`delay()`-based loop fighting the fade |
| Fade looks jumpy at the dim end | normal perception (gamma); apply a curve if you care |

---

## TL;DR

A digital pin can fake analog output by switching fast and varying the **duty cycle** —
that's **PWM**. `analogWrite(pin, 0..255)` sets the duty on a `~` PWM pin (3,5,6,9,10,11
on the Uno). Your eye averages the pulses into a brightness. Same idea drives motor speed,
servos, and tone.
