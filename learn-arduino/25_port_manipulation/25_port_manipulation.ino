/*
 * Challenge #25: Direct Port Manipulation & Bitwise Ops
 *
 * Sketch Name:
 *
 *   25_port_manipulation
 *
 * Objective:
 *
 *   Control the three LEDs by writing DIRECTLY to the
 *   AVR's hardware registers -- no pinMode(), no
 *   digitalWrite() -- and learn the bitwise operations
 *   that make it possible.
 *
 * Background -- pins are bits in registers:
 *
 *   digitalWrite() / pinMode() are convenient wrappers.
 *   Underneath, each group of pins is an 8-bit hardware
 *   register. On the Uno, digital pins 8..13 live in
 *   PORT B:
 *
 *       pin 8  -> PB0      pin 11 -> PB3
 *       pin 9  -> PB1      pin 12 -> PB4
 *       pin 10 -> PB2      pin 13 -> PB5
 *
 *   Three registers control PORT B (all 8 bits wide):
 *
 *       DDRB  - Data Direction: 1 = output, 0 = input
 *       PORTB - Output state:   1 = HIGH,   0 = LOW
 *       PINB  - Input state:    read pin levels
 *
 *   Setting a pin = setting a bit.
 *
 * Bitwise idioms (for bit n):
 *
 *       set bit:    REG |=  (1 << n);
 *       clear bit:  REG &= ~(1 << n);
 *       toggle bit: REG ^=  (1 << n);
 *       test bit:   (REG >> n) & 1;
 *
 *   Write SEVERAL bits at once without disturbing the
 *   others (read-modify-write):
 *
 *       REG = (REG & ~mask) | (value & mask);
 *
 * Hardware:
 *
 *   LED1 -> Pin 8   (PB0)
 *   LED2 -> Pin 9   (PB1)
 *   LED3 -> Pin 10  (PB2)
 *
 * Behaviour:
 *
 *   A 3-bit binary counter on the LEDs. Every 500ms the
 *   count goes 0,1,2,...,7,0,... shown in binary, where
 *   LED1 = bit 0 (least significant), LED3 = bit 2:
 *
 *       count  LED3 LED2 LED1
 *       0       0    0    0
 *       1       0    0    1
 *       2       0    1    0
 *       3       0    1    1
 *       ...
 *       7       1    1    1
 *
 * Requirements:
 *
 *   - Set pins 8,9,10 as OUTPUT by writing DDRB directly
 *     (no pinMode()).
 *
 *   - Keep a counter 0..7; increment every 500ms using
 *     millis() (no delay()).
 *
 *   - Show the counter's low 3 bits on PORTB bits 0..2
 *     using bitwise ops -- and DO NOT clobber the other
 *     PORTB bits (pins 11-13). Use the read-modify-write
 *     idiom with a mask.
 *
 * Rules:
 *
 *   - No pinMode(), no digitalWrite(), no digitalRead().
 *     Registers only.
 *   - Use millis(). Do not use delay().
 *   - Never write PORTB or DDRB as a whole byte
 *     (e.g. `PORTB = counter;`) -- that would stomp the
 *     other pins. Always mask.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - Bitwise operators ( | & ^ ~ << >> )
 *     - Bit masks
 *     - Read-modify-write
 *     - Binary / hex literals
 *
 *   Embedded:
 *     - Hardware registers (DDR / PORT / PIN)
 *     - Memory-mapped I/O
 *     - What digitalWrite()/pinMode() really do
 *     - Why direct port access is faster & how it's
 *       used for tight timing
 *
 * Learning Goal:
 *
 *   See that a "pin" is just a bit in a register, and
 *   that every digitalWrite() you've written compiles
 *   down to exactly the bitwise ops above. This is the
 *   floor of embedded programming.
 *
 * Stretch Goals (optional):
 *
 *   - Re-do the Knight Rider scanner (Challenge 8) using
 *     only PORTB writes.
 *   - Toggle pin 13 with PORTB ^= (1 << PB5) and compare
 *     against digitalWrite for speed.
 *   - Read a button on PINB with INPUT_PULLUP set via
 *     PORTB (input + pull-up = DDR 0, PORT 1).
 */

#include <Arduino.h>
#include <avr/io.h>

// Uno: pins 8, 9, 10 are PORTB bits PB0, PB1, PB2.
const uint8_t LED_MASK = (1 << PB0) | (1 << PB1) | (1 << PB2); // 0b00000111

unsigned long timer = 0;
const int STEP_MS = 500;
uint8_t counter = 0; // 0..7, shown in binary on the 3 LEDs

void setup() {
  // TODO: set pins 8,9,10 as OUTPUT via DDRB (one masked write, no pinMode).
  DDRB |= LED_MASK;
}

void loop() {
  // TODO (non-blocking, every STEP_MS):
  //   1. increment counter, wrapping back to 0 after 7
  //   2. show its low 3 bits on PORTB bits 0..2 using read-modify-write,
  //      leaving the other PORTB bits untouched.
  if (millis() - timer >= STEP_MS) {
    timer += STEP_MS;
    PORTB = (PORTB & ~LED_MASK) | (counter & LED_MASK);
    counter++;
    if (counter > 7) {
      counter = 0;
    }
  }
}
