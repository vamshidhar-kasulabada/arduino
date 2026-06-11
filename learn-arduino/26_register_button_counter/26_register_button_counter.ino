/*
 * Challenge #26: Register-Only Button Input (PINB + Pull-Ups)
 *
 * Sketch Name:
 *
 *   26_register_button_counter
 *
 * Objective:
 *
 *   Read a push-button by reading the AVR's INPUT register
 *   (PINB) directly -- no pinMode(), no digitalRead() -- and
 *   enable the chip's INTERNAL PULL-UP through the registers.
 *   Each press advances the 3-bit binary counter from
 *   Challenge 25.
 *
 * Background -- the OTHER direction of the same registers:
 *
 *   In Challenge 25 you drove pins by WRITING registers. To
 *   read a pin you use the same three Port B registers the
 *   other way round:
 *
 *       DDRB  - Data Direction: 0 = INPUT, 1 = output
 *       PORTB - for an INPUT pin: 1 = enable internal pull-up
 *       PINB  - READ this to get the live level of each pin
 *
 *   A bare input pin "floats" and reads random noise, so we
 *   turn on the internal PULL-UP resistor: it gently holds the
 *   pin HIGH (reads 1) when nothing else drives it. Wire the
 *   button between the pin and GND, and pressing it pulls the
 *   pin LOW (reads 0). That is "active-low": released = 1,
 *   pressed = 0.
 *
 *   INPUT + PULL-UP, set with registers, is:
 *       DDRB  bit = 0   (input)
 *       PORTB bit = 1   (pull-up on)
 *
 *   Read one bit n:   (PINB >> n) & 1   -> 0 or 1
 *
 * Hardware:
 *
 *   LED1 -> Pin 8   (PB0)        Button -> Pin 12 (PB4)
 *   LED2 -> Pin 9   (PB1)                  wired pin 12 --[btn]-- GND
 *   LED3 -> Pin 10  (PB2)                  (no external resistor;
 *                                           internal pull-up is used)
 *
 * Behaviour:
 *
 *   The 3 LEDs show a counter 0..7 in binary (LED1 = bit 0).
 *   Every time you PRESS the button (a fresh press, not held),
 *   the counter increments by one and wraps 7 -> 0.
 *
 * Requirements:
 *
 *   - Configure PB0,PB1,PB2 as OUTPUT via DDRB (as in #25).
 *   - Configure PB4 as INPUT with the INTERNAL PULL-UP, using
 *     registers only:  DDRB bit 4 = 0,  PORTB bit 4 = 1.
 *   - Read the button from PINB (not PORTB).
 *   - DEBOUNCE the button with millis() (ignore bouncing
 *     changes faster than DEBOUNCE_MS).
 *   - Act on the FALLING edge only (1 -> 0): one increment per
 *     physical press, no matter how long it's held.
 *   - Show the counter on PB0..PB2 with read-modify-write so
 *     you never disturb PB4 or the other Port B bits.
 *
 * Rules:
 *
 *   - No pinMode(), no digitalWrite(), no digitalRead().
 *     Registers only.
 *   - Use millis(). Do not use delay().
 *   - Never write PORTB/DDRB as a whole byte -- always mask.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Reading bits:  (PINB >> n) & 1
 *     - Edge detection (compare to previous stable level)
 *     - Bit masks, read-modify-write (from #25)
 *
 *   Embedded:
 *     - Input registers (PIN) vs output registers (PORT)
 *     - Floating inputs and why they're a problem
 *     - Internal pull-up resistors, set via registers
 *     - Active-low logic
 *     - Debouncing a mechanical switch
 *
 * Learning Goal:
 *
 *   See that reading a pin is just reading a bit of PINB, and
 *   that INPUT_PULLUP is nothing more than DDR=0 + PORT=1. You
 *   now know BOTH directions of port manipulation.
 *
 * Stretch Goals (optional):
 *
 *   - Add a second button on PB3 (pin 11) that DECREMENTS.
 *   - Hold-to-repeat: if held, auto-increment every 300ms.
 *   - Read both buttons in one PINB read and decode with masks.
 */

#include <Arduino.h>
#include <avr/io.h>

// LEDs: pins 8,9,10 = PORTB bits PB0,PB1,PB2 (outputs).
const uint8_t LED_MASK = (1 << PB0) | (1 << PB1) | (1 << PB2); // 0b00000111

// Button: pin 12 = PORTB bit PB4 (input, internal pull-up).
const uint8_t BTN_BIT = PB4;

uint8_t counter = 0; // 0..7, shown in binary on the 3 LEDs

// Debounce + edge-detection state.
// With a pull-up, RELEASED reads 1 (HIGH) and PRESSED reads 0 (LOW).
const unsigned long DEBOUNCE_MS = 30;
unsigned long lastChangeAt = 0;  // WHEN the raw reading last changed
uint8_t lastReadLevel   = 1;     // previous RAW reading (to spot a change)
uint8_t lastStableLevel = 1;     // last CONFIRMED (debounced) level we acted on

void setup() {
  // TODO 1: make PB0,PB1,PB2 outputs in DDRB (one masked write, as in #25).

  // TODO 2: make PB4 an INPUT with the internal PULL-UP:
  //           - clear its DDRB bit  (DDRB  &= ~(1 << BTN_BIT))  -> input
  //           - set   its PORTB bit (PORTB |=  (1 << BTN_BIT))  -> pull-up on

  // TODO 3: show the starting counter (0) on the LEDs (read-modify-write).
  DDRB |= LED_MASK;
  PORTB |= (1 << BTN_BIT);
  DDRB &= ~(1 << BTN_BIT);
}

void loop() {
  // Read the button's RAW live level (it may be mid-bounce):
  //   1 = released, 0 = pressed  (active-low, thanks to the pull-up).
  uint8_t level = (PINB >> BTN_BIT) & 1;

  // (A) Every time the RAW reading changes, restart the steadiness timer.
  //     Each bounce flicker lands here and keeps pushing the clock forward,
  //     so the timer only ever "completes" once the pin has gone quiet.
  if (level != lastReadLevel) {
    lastChangeAt = millis();
    lastReadLevel = level;
  }

  // (B) Once the reading has held steady for DEBOUNCE_MS, trust it as the
  //     real, settled level -- now run the edge logic on it.
  if (millis() - lastChangeAt >= DEBOUNCE_MS) {
    // Falling edge on the SETTLED level (1 -> 0) = one genuine press.
    if (lastStableLevel == 1 && level == 0) {
      counter = (counter + 1) & 7;
      PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);
    }
    lastStableLevel = level;  // remember the confirmed level for next time
  }
}
