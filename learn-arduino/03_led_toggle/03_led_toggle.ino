/*
 * Challenge #3: LED Toggle
 *
 * Sketch Name:
 *
 *   03_led_toggle
 *
 * Objective:
 *
 *   Toggle an LED whenever a button is pressed.
 *
 * Hardware:
 *
 *   LED    -> Pin 13
 *   Button -> Pin 8
 *
 * Requirements:
 *
 *   - Detect a button press event.
 *   - Toggle the LED state.
 *   - Avoid repeated toggles while
 *     holding the button.
 *
 * Concepts Practiced:
 *
 *   - Edge Detection
 *   - Previous State Tracking
 *   - Button Bounce Discovery
 *   - State Variables
 */

#include <Arduino.h>

bool ledOn = false;
int previousPin8State = HIGH;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(8, INPUT_PULLUP);
}

void loop() {
  int currentPin8State = digitalRead(8);
  if (previousPin8State == HIGH && currentPin8State == LOW) {
    ledOn = !ledOn;
  }
  previousPin8State = currentPin8State;
  digitalWrite(13, ledOn);
}
