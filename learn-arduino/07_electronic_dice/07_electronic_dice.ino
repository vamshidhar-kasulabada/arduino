/*
 * Challenge #7: Electronic Dice Simulator
 *
 * Objective:
 * Build an electronic dice using 3 LEDs and a push button.
 *
 * Hardware:
 *
 *   LED1 -> Pin 8
 *   LED2 -> Pin 9
 *   LED3 -> Pin 10
 *   Button -> Pin 2 (INPUT_PULLUP)
 *
 * Functional Requirements:
 *
 *   1. System starts in WAITING state.
 *   2. All LEDs remain OFF while waiting.
 *   3. When the button is pressed:
 *        - Enter the ROLLING state.
 *        - Simulate a rolling dice animation.
 *   4. During ROLLING:
 *        - Display a new random value every 300ms.
 *        - Continue rolling for 5 seconds.
 *   5. After 5 seconds:
 *        - Generate a final random dice value.
 *        - Enter the RESULT state.
 *   6. During RESULT:
 *        - Display the final dice value.
 *        - Keep the result visible for 10 seconds.
 *   7. After 10 seconds:
 *        - Return to WAITING state.
 *        - Turn all LEDs OFF.
 *
 * Dice Values:
 *
 *   Random values must be generated in the range:
 *
 *     1 to 6
 *
 * Binary LED Representation:
 *
 *   Value  Binary   LED3 LED2 LED1
 *   -----  ------   ---- ---- ----
 *     1     001      OFF OFF ON
 *     2     010      OFF ON  OFF
 *     3     011      OFF ON  ON
 *     4     100      ON  OFF OFF
 *     5     101      ON  OFF ON
 *     6     110      ON  ON  OFF
 *
 * Rules:
 *
 *   - Do not use delay().
 *   - Use millis() for all timing operations.
 *   - Use edge detection to detect button presses.
 *   - Ignore button presses while already rolling
 *     or displaying a result.
 *   - Use random() to generate dice values.
 *   - Use randomSeed() to initialize the random generator.
 *
 * Suggested State Machine:
 *
 *   WAITING
 *      ↓ button press
 *   ROLLING
 *      ↓ 5 seconds elapsed
 *   RESULT
 *      ↓ 10 seconds elapsed
 *   WAITING
 *
 * Concepts Practiced:
 *
 *   - State Machines
 *   - Enums
 *   - Edge Detection
 *   - Random Number Generation
 *   - Binary Representation
 *   - Bit Manipulation
 *   - Non-blocking Programming with millis()
 */



#include <Arduino.h>

enum State { WAITING, ROLLING, RESULT };

State game = WAITING;
uint8_t prevBtnState = HIGH;
uint8_t value;
unsigned long time;
unsigned long tempTime;

void displayValue(uint8_t value) {
  digitalWrite(8, value & 1);
  digitalWrite(9, (value >> 1) & 1);
  digitalWrite(10, (value >> 2) & 1);
}

void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  randomSeed(analogRead(A0));
}

void loop() {
  uint8_t currentBtnState = digitalRead(2);
  if (prevBtnState == HIGH && currentBtnState == LOW && game == WAITING) {
    game = ROLLING;
    time = millis();
    tempTime = time;
  }
  prevBtnState = currentBtnState;

  switch (game) {
  case WAITING:
    displayValue(0);
    break;
  case ROLLING:
    if (millis() - time >= 5000) {
      value = random(1, 7);
      tempTime = millis();
      game = RESULT;
    } else if (millis() - tempTime >= 300) {
      byte rand = random(1, 7);
      displayValue(rand);
      tempTime = millis();
    }
    break;
  case RESULT:
    if (millis() - tempTime >= 10000) {
      game = WAITING;
    } else {
      displayValue(value);
    }
    break;
  }
}
