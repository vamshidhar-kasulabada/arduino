/*
 * Challenge #1: Blink LED
 *
 * Sketch Name:
 *
 *   01_blink_led
 *
 * Objective:
 *
 *   Blink an LED continuously.
 *
 * Hardware:
 *
 *   LED -> Pin 13
 *
 * Behaviour:
 *
 *   LED ON  for 1 second
 *   LED OFF for 1 second
 *
 *   Repeat forever.
 *
 * Requirements:
 *
 *   - Configure the LED pin as OUTPUT.
 *   - Turn the LED ON.
 *   - Wait for 1 second.
 *   - Turn the LED OFF.
 *   - Wait for 1 second.
 *
 * Concepts Practiced:
 *
 *   - pinMode()
 *   - digitalWrite()
 *   - delay()
 *   - Arduino setup() and loop()
 */


#include <Arduino.h>

void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
