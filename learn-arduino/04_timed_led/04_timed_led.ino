/*
 * Challenge #4: Timed LED
 *
 * Sketch Name:
 *
 *   04_timed_led
 *
 * Objective:
 *
 *   Turn an LED ON for 5 seconds whenever
 *   a button is pressed.
 *
 * Hardware:
 *
 *   LED    -> Pin 13
 *   Button -> Pin 8
 *
 * Behaviour:
 *
 *   Button Pressed
 *       ↓
 *   LED ON
 *       ↓
 *   Wait 5 Seconds
 *       ↓
 *   LED OFF
 *
 * Requirements:
 *
 *   - Use millis().
 *   - Do not use delay().
 *
 * Concepts Practiced:
 *
 *   - millis()
 *   - Non-blocking Timing
 *   - Time Measurement
 *   - Event Driven Logic
 */

#include <Arduino.h>

unsigned long buttonLastPressedAt = 0;
int prevPin8state = HIGH;
int ledState = LOW;
uint16_t time = 5000;

void setup() {
  pinMode(8, INPUT_PULLUP);
  pinMode(13, OUTPUT);
}

void loop() {
  int currentPin8State = digitalRead(8);
  if (prevPin8state == HIGH && currentPin8State == LOW) {
    buttonLastPressedAt = millis();
    ledState = HIGH;
  }
  prevPin8state = currentPin8State;
  if (millis() - buttonLastPressedAt >= time) {
    ledState = LOW;
  }
  digitalWrite(13, ledState);
}
