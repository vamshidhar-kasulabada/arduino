/*
 * Challenge #5: Reaction Timer Game
 *
 * Sketch Name:
 *
 *   05_reaction_timer
 *
 * Objective:
 *
 *   Measure how quickly the player reacts
 *   after an LED turns ON.
 *
 * Hardware:
 *
 *   LED    -> Pin 13
 *   Button -> Pin 8
 *
 * Game Flow:
 *
 *   Press Button
 *       ↓
 *   Random Delay (2-8 Seconds)
 *       ↓
 *   LED Turns ON
 *       ↓
 *   Player Presses Button
 *       ↓
 *   Display Reaction Time
 *
 * Early Press Rule:
 *
 *   If the button is pressed before
 *   the LED turns ON:
 *
 *       "Too Early!"
 *
 * Requirements:
 *
 *   - Use millis().
 *   - Use random().
 *   - Use randomSeed().
 *   - Use Serial Monitor.
 *
 * Concepts Practiced:
 *
 *   - State Machines
 *   - Random Numbers
 *   - Timing Measurements
 *   - Human Interaction
 */

#include <Arduino.h>

unsigned long time;
int gameState = 0;
int prevBtnState = HIGH;
unsigned long gameStartTime = 0;
unsigned long btnPressedTime = 0;

void setup() {
  pinMode(8, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  time = random(2000, 8000);
}

void loop() {
  if (gameState == 0) {
    digitalWrite(13, LOW);
    int currentBtnState = digitalRead(8);
    if (prevBtnState == HIGH && currentBtnState == LOW) {
      gameState = 1;
      gameStartTime = millis();
      time = random(2000, 8000);
      Serial.println("Game Started, wait for led to switch on");
    }
    prevBtnState = currentBtnState;
  } else if (gameState == 1) {
    if (millis() - gameStartTime >= time) {
      digitalWrite(13, HIGH);
      Serial.println("LED ON");
      btnPressedTime = millis();
      gameState = 2;
    } else {
      int currentBtnState = digitalRead(8);
      if (prevBtnState == HIGH && currentBtnState == LOW) {
        Serial.println("Too Early!!");
        gameState = 0;
      }
      prevBtnState = currentBtnState;
    }
  } else if (gameState == 2) {
    int currentBtnState = digitalRead(8);
    if (prevBtnState == HIGH && currentBtnState == LOW) {
      unsigned long reactionTime = millis() - btnPressedTime;
      Serial.print("Your Reaction Time is: ");
      Serial.println(reactionTime);
      gameState = 0;
      digitalWrite(13, LOW);
    }
    prevBtnState = currentBtnState;
  }
}
