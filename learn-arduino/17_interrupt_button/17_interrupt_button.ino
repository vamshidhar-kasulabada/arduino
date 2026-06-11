/*
 * Challenge #17: Interrupt Button
 *
 * Sketch Name:
 *
 *   17_interrupt_button
 *
 * Objective:
 *
 *   Use an external interrupt to detect
 *   button presses instead of polling.
 *
 * Hardware:
 *
 *   Button -> Pin 2
 *   LED    -> Pin 8
 *
 * Behaviour:
 *
 *   Each button press should toggle
 *   the LED.
 *
 * Requirements:
 *
 *   - Use attachInterrupt().
 *   - Use INPUT_PULLUP.
 *   - Use FALLING edge detection.
 *   - Do not poll the button.
 *
 * Rules:
 *
 *   - The interrupt handler should
 *     execute quickly.
 *
 *   - Do not use delay() inside
 *     the interrupt.
 *
 * Concepts Practiced:
 *
 *   Embedded:
 *     - Interrupts
 *     - ISR
 *     - Event Driven Programming
 *
 * Learning Goal:
 *
 *   Understand the difference between
 *   polling and hardware interrupts.
 */

#include <Arduino.h>

const int buttonPin = 2;
volatile bool buttonWasPressed = false;

void buttonPressed() { buttonWasPressed = true; }

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);
}

void loop() {
  if (buttonWasPressed) {
    Serial.println("Pressed!");
    buttonWasPressed = false;
  }
}
