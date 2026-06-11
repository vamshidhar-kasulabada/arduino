/*
 * Challenge #2: Button Input
 *
 * Sketch Name:
 *
 *   02_button_input
 *
 * Objective:
 *
 *   Read a push button and print its state
 *   to the Serial Monitor.
 *
 * Hardware:
 *
 *   Button -> Pin 8
 *
 * Requirements:
 *
 *   - Configure the button using INPUT_PULLUP.
 *   - Read the button state.
 *   - Print HIGH or LOW to Serial Monitor.
 *
 * Behaviour:
 *
 *   Button Released -> HIGH
 *   Button Pressed  -> LOW
 *
 * Concepts Practiced:
 *
 *   - digitalRead()
 *   - INPUT_PULLUP
 *   - Serial.begin()
 *   - Serial.println()
 */

#include <Arduino.h>


void setup() {
  Serial.begin(9600);
  pinMode(8,INPUT_PULLUP);
}

void loop(){
  if (digitalRead(8) == LOW){
    Serial.println("HIGH");
  }else{
    Serial.println("LOW");
  }
}
