/*
 * Challenge #24: PWM LED Fade (Breathing LED)
 *
 * Sketch Name:
 *
 *   24_pwm_led_fade
 *
 * Objective:
 *
 *   Smoothly fade an LED up and down (a "breathing"
 *   effect) using PWM -- without delay().
 *
 *   So far every LED has been ON or OFF. This one
 *   lives at every brightness in between.
 *
 * Background -- what is PWM?
 *
 *   digitalWrite() can only do HIGH or LOW. To get
 *   "half brightness" the Arduino uses PWM (Pulse
 *   Width Modulation): it switches the pin on and off
 *   very fast, and varies the fraction of time it
 *   stays on (the "duty cycle"). 50% on => looks half
 *   as bright; 10% on => dim.
 *
 *       analogWrite(pin, value);   // value 0..255
 *       //   0   = always off
 *       //   127 = ~50% duty (half bright)
 *       //   255 = always on
 *
 *   PWM is the same trick used to set motor speed,
 *   servo position, and audio tone.
 *
 * Hardware:
 *
 *   LED -> Pin 9
 *
 *   IMPORTANT: analogWrite only works on PWM pins,
 *   marked with a ~ on the board. On the Uno those are
 *   3, 5, 6, 9, 10, 11. Pin 8 is NOT a PWM pin, so use
 *   pin 9 here.
 *
 * Behaviour:
 *
 *   Brightness ramps 0 -> 255, then 255 -> 0, forever,
 *   smoothly. Like the LED is breathing.
 *
 * Requirements:
 *
 *   - Use analogWrite() for brightness.
 *
 *   - Use millis() to step the brightness at a fixed
 *     rate (e.g. every 10ms). Do NOT use delay().
 *
 *   - Track:
 *       * current brightness (0..255)
 *       * direction (+1 going up, -1 going down)
 *
 *   - When brightness reaches 255 or 0, flip direction.
 *
 * Rules:
 *
 *   - The LED must be on a PWM (~) pin.
 *   - No delay(). The fade must be non-blocking so other
 *     work could run alongside it.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - State variables (brightness, direction)
 *     - Clamping / bounds logic
 *
 *   Embedded:
 *     - PWM and duty cycle
 *     - Analog-style output from a digital pin
 *     - Non-blocking ramps (reusing the millis pattern)
 *
 * Learning Goal:
 *
 *   Understand PWM as the foundation of analog-ish
 *   control: brightness, motor speed, servo angle, and
 *   tone all come from varying a duty cycle.
 *
 * Stretch Goals (optional):
 *
 *   - Wrap it in a class:  PwmLed / Fader  with a
 *     update() method (hardware abstraction, like your
 *     Led class).
 *   - Reuse Challenge 23: make a FadeTask : public Task
 *     and run it inside your polymorphic Scheduler.
 *   - Fade two LEDs at different speeds at the same time.
 */

#include <Arduino.h>

const uint8_t LED_PIN = 9; // PWM (~) pin — pin 8 would NOT work for analogWrite
const int STEP_MS = 10;    // change brightness every 10ms (controls fade speed)

unsigned long timer = 0;
int brightness = 0;
int direction = 1; // +1 = getting brighter, -1 = getting dimmer

void setup() { pinMode(LED_PIN, OUTPUT); }

void loop() {
  if (millis() - timer >= STEP_MS) {
    timer += STEP_MS;
    analogWrite(LED_PIN, brightness);
    brightness += direction;
    if (brightness >= 255) {
      direction = -1;
      brightness = 255;
    } else if (brightness <= 0) {
      direction = 1;
      brightness = 0;
    }
  }
}
