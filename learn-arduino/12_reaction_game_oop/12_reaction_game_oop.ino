/*
 * Challenge #12: Reaction Timer Game (OOP Version)
 *
 * Sketch Name:
 *
 *   12_reaction_game_oop
 *
 * Objective:
 *
 *   Rebuild the Reaction Timer Game using the
 *   reusable Led and Button classes created in
 *   previous challenges.
 *
 * Hardware:
 *
 *   LED    -> Pin 8
 *   Button -> Pin 2
 *
 * Game Flow:
 *
 *   1. System starts in WAITING state.
 *   2. Player presses the button to start.
 *   3. System waits for a random delay
 *      between 2 and 8 seconds.
 *   4. LED turns ON.
 *   5. Player must press the button as
 *      quickly as possible.
 *   6. Reaction time is measured and
 *      displayed on the Serial Monitor.
 *   7. Game returns to WAITING state.
 *
 * Early Press Rule:
 *
 *   If the player presses the button before
 *   the LED turns ON:
 *
 *       "Too Early!"
 *
 *   should be displayed and the game should
 *   return to WAITING state.
 *
 * Requirements:
 *
 *   - Use millis() for all timing.
 *   - Do not use delay().
 *   - Use random() for the wait time.
 *   - Use randomSeed().
 *   - Use a state machine.
 *
 * Suggested States:
 *
 *   WAITING
 *   WAITING_FOR_LED
 *   MEASURING_REACTION
 *
 * Rules:
 *
 *   - Do not use digitalRead() directly.
 *   - Do not use digitalWrite() directly.
 *   - Interact with hardware only through:
 *
 *         button.wasPressed()
 *         led.on()
 *         led.off()
 *
 *   - Button debounce logic must remain
 *     inside the Button class.
 *
 *   - LED control logic must remain
 *     inside the Led class.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Classes
 *     - Object Reuse
 *     - Header Files
 *     - Encapsulation
 *     - Composition
 *
 *   Embedded Systems:
 *     - State Machines
 *     - Event-Driven Programming
 *     - Non-blocking Timing
 *     - Random Number Generation
 *     - Human Input Handling
 *
 * Learning Goal:
 *
 *   Demonstrate that good abstractions allow
 *   an old application to be rebuilt with
 *   cleaner and more maintainable code.
 *
 *   The game logic should focus on:
 *
 *       states
 *       timers
 *       reaction measurement
 *
 *   rather than:
 *
 *       digitalRead()
 *       digitalWrite()
 *       debounce logic
 *
 *   because those responsibilities are now
 *   handled by reusable classes.
 */

#include "Button.h"
#include "Led.h"
#include <Arduino.h>

enum GameState {
  WAITING_FOR_BTN_PRESS,
  WAITING_FOR_LED,
  WAITING_FOR_REACTION,
};

Led led(8);
Button button(2);
GameState gameState = WAITING_FOR_BTN_PRESS;
int value = 0;
unsigned long btnPressedAt = 0;
unsigned long reactionTime = 0;
unsigned long ledOnAt = 0;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void loop() {
  switch (gameState) {
  case WAITING_FOR_BTN_PRESS:
    if (button.wasPressed()) {
      Serial.println("BUTTON PRESSED");
      gameState = WAITING_FOR_LED;
      btnPressedAt = millis();
      value = random(2000, 8000);
    }
    break;
  case WAITING_FOR_LED:
    if (millis() - btnPressedAt >= value) {
      ledOnAt = millis();
      led.on();
      gameState = WAITING_FOR_REACTION;
    } else if (button.wasPressed()) {
      Serial.println("Too Early!");
      gameState = WAITING_FOR_BTN_PRESS;
    }
    break;
  case WAITING_FOR_REACTION:
    if (button.wasPressed()) {
      btnPressedAt = millis();
      Serial.println("Reaction Time: ");
      reactionTime = btnPressedAt - ledOnAt;
      Serial.print(reactionTime);
      Serial.println("ms");
      gameState = WAITING_FOR_BTN_PRESS;
      led.off();
    }
    break;
  }
}
