/*
 * Challenge #15: Multi LED Controller
 *
 * Sketch Name:
 *
 *   15_multi_led_controller
 *
 * Objective:
 *
 *   Control multiple LEDs through Serial
 *   commands.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *
 * Commands:
 *
 *   ON 1
 *   OFF 2
 *   TOGGLE 3
 *   STATUS
 *
 * Examples:
 *
 *   ON 1
 *     -> Turns LED1 ON
 *
 *   OFF 2
 *     -> Turns LED2 OFF
 *
 *   TOGGLE 3
 *     -> Toggles LED3
 *
 *   STATUS
 *     -> Prints the state of all LEDs
 *
 * Example Output:
 *
 *   LED1 : ON
 *   LED2 : OFF
 *   LED3 : ON
 *
 * Requirements:
 *
 *   - Use an array of Led objects.
 *
 *   - Read commands from Serial Monitor.
 *
 *   - Support:
 *
 *         ON n
 *         OFF n
 *         TOGGLE n
 *         STATUS
 *
 *   - Validate LED numbers.
 *
 *   Example:
 *
 *       ON 4
 *
 *   should print:
 *
 *       INVALID LED
 *
 * Rules:
 *
 *   - Do not use digitalWrite() directly.
 *
 *   - Use Led class methods only.
 *
 *   - Do not create separate variables:
 *
 *         redLed
 *         yellowLed
 *         greenLed
 *
 *   Use an array instead.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Arrays of Objects
 *     - String Parsing
 *     - Index Validation
 *     - Reusable Code
 *
 *   Embedded:
 *     - Serial Command Processing
 *     - Runtime Hardware Control
 *     - Component Scaling
 *
 * Learning Goal:
 *
 *   Move from controlling:
 *
 *       one LED
 *
 *   to controlling:
 *
 *       N LEDs
 *
 *   using the same code structure.
 */

#include "Led.h"
#include <Arduino.h>

Led leds[] = {Led(8), Led(9), Led(10)};
int size = sizeof(leds) / sizeof(leds[0]);

int getLedNumber(String s) {
  int numIndex = s.lastIndexOf(" ");
  int ledNum = s.substring(numIndex).toInt();
  return ledNum;
};

bool isNumValid(int n) { return n >= 1 && n <= size; }

void setup() { Serial.begin(9600); };
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    int ledNum = getLedNumber(command);
    if (isNumValid(ledNum)) {
      if (command.startsWith("ON")) {
        leds[ledNum - 1].on();
      } else if (command.startsWith("OFF")) {
        leds[ledNum - 1].off();
      } else if (command.startsWith("TOGGLE")) {
        leds[ledNum - 1].toggle();
      }
    } else if (command.equals("STATUS")) {
      for (int i = 1; i <= size; i++) {
        Serial.print("LED");
        Serial.print(i);
        Serial.print(" : ");
        if (leds[i-1].isOn()) {
          Serial.println("ON");
        } else {
          Serial.println("OFF");
        }
      }
    } else {
      Serial.println("INVALID LED");
    }
  }
};
