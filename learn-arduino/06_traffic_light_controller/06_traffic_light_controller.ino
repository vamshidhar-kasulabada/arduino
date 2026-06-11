/*
 * Challenge #6: Traffic Light Controller with Pedestrian Crossing
 *
 * Objective:
 * Implement a traffic light controller using a finite state machine and
 * non-blocking timers (millis()).
 *
 * Traffic Light Sequence:
 *
 *   RED          -> 5 seconds
 *   RED_YELLOW   -> 2 seconds
 *   GREEN        -> 5 seconds
 *   YELLOW       -> 2 seconds
 *   Repeat forever
 *
 * Hardware:
 *
 *   Red LED    -> Pin 8
 *   Yellow LED -> Pin 9
 *   Green LED  -> Pin 10
 *   Push Button -> Pin 2 (INPUT_PULLUP)
 *
 * Rules:
 *
 *   - Do not use delay()
 *   - Use millis() for timing
 *   - Use a state machine (enum)
 *   - Each state should manage its own duration
 *
 * Pedestrian Crossing Feature:
 *
 *   When the pedestrian button is pressed:
 *
 *     - Do NOT interrupt the current traffic state.
 *     - Record that a pedestrian is waiting.
 *     - Allow the current traffic cycle to continue normally.
 *     - When the traffic light reaches RED,
 *       enter a dedicated PEDESTRIAN_RED state.
 *     - Keep RED active for an additional 5 seconds
 *       to allow pedestrians to cross safely.
 *     - After the pedestrian crossing period,
 *       resume the normal traffic sequence.
 *
 * Example:
 *
 *   GREEN
 *     ↓ (button pressed)
 *   GREEN completes
 *     ↓
 *   YELLOW completes
 *     ↓
 *   RED
 *     ↓
 *   PEDESTRIAN_RED (5 seconds)
 *     ↓
 *   RED_YELLOW
 *     ↓
 *   GREEN
 *
 * Concepts Practiced:
 *
 *   - Digital Outputs
 *   - INPUT_PULLUP
 *   - Edge Detection
 *   - Enums
 *   - State Machines
 *   - Non-blocking Programming
 *   - Event-driven Design
 *   - Timers using millis()
 */



#include <Arduino.h>

enum TrafficLightState { RED, GREEN, YELLOW, RED_YELLOW, PEDESTRIAN_RED };
unsigned long startTime = 0;
TrafficLightState state = RED;
const short redPin = 8;
const short yellowPin = 9;
const short greenPin = 10;

int prevBtnState = HIGH;
bool pedestrianWaiting = false;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  int currentBtnState = digitalRead(2);
  if (prevBtnState == HIGH && currentBtnState == LOW) {
    pedestrianWaiting = true;
  }
  prevBtnState = currentBtnState;
  switch (state) {
  case RED:
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
    if (millis() - startTime >= 5000) {
      if (pedestrianWaiting) {
        state = PEDESTRIAN_RED;
      } else {
        state = RED_YELLOW;
      }
      startTime = millis();
    }
    break;
  case RED_YELLOW:
    digitalWrite(yellowPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    if (millis() - startTime >= 2000) {
      state = GREEN;
      startTime = millis();
    }
    break;
  case GREEN:
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
    if (millis() - startTime >= 5000) {
      state = YELLOW;
      startTime = millis();
    }
    break;
  case YELLOW:
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, HIGH);
    digitalWrite(greenPin, LOW);
    if (millis() - startTime >= 2000) {
      state = RED;
      startTime = millis();
    }
    break;
  case PEDESTRIAN_RED:
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
    if (millis() - startTime >= 5000) {
      pedestrianWaiting = false;
      state = RED_YELLOW;
      startTime = millis();
    }
    break;
  }
}
