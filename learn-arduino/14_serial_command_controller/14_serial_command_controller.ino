/*
 * Challenge #14: Serial Command Controller
 *
 * Sketch Name:
 *
 *   14_serial_command_controller
 *
 * Objective:
 *
 *   Control an LED by sending commands from
 *   the Serial Monitor.
 *
 * Hardware:
 *
 *   LED -> Pin 8
 *
 * Example Commands:
 *
 *   ON
 *   OFF
 *   TOGGLE
 *   STATUS
 *
 * Expected Behaviour:
 *
 *   ON
 *     -> Turns LED ON
 *
 *   OFF
 *     -> Turns LED OFF
 *
 *   TOGGLE
 *     -> Toggles LED state
 *
 *   STATUS
 *     -> Prints current LED state
 *
 * Example Session:
 *
 *   > ON
 *   LED ON
 *
 *   > STATUS
 *   LED IS ON
 *
 *   > TOGGLE
 *   LED OFF
 *
 *   > STATUS
 *   LED IS OFF
 *
 * Requirements:
 *
 *   - Use your Led class.
 *   - Read commands from Serial Monitor.
 *   - Commands should be case-sensitive.
 *
 *   Unknown commands should print:
 *
 *       UNKNOWN COMMAND
 *
 * Rules:
 *
 *   - Do not use digitalWrite() directly.
 *   - Use Led class methods only.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Strings
 *     - Conditional Logic
 *     - Object Reuse
 *
 *   Embedded:
 *     - UART Communication
 *     - Serial Monitor
 *     - Command Processing
 *     - Runtime Control
 *
 * Learning Goal:
 *
 *   Learn how a microcontroller communicates
 *   with a computer and how commands can be
 *   used to control hardware at runtime.
 */

#include "Led.h"
#include <Arduino.h>

Led led(8);

void setup() { Serial.begin(9600); }
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equals("ON")) {
      led.on();
      Serial.println("LED ON");
    } else if (command.equals("OFF")) {
      led.off();
      Serial.println("LED OFF");
    } else if (command.equals("TOGGLE")) {
      led.toggle();
    } else if (command.equals("STATUS")) {
      if (led.isOn()) {
        Serial.println("LED is ON");
      } else {
        Serial.println("LED is OFF");
      }
    } else {
      Serial.println("UNKNOWN COMMAND");
    }
  }
}
