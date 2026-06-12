/*
 * Challenge #27: Hardware Timer Interrupt (Timer1, CTC Mode)
 *
 * Sketch Name:
 *
 *   27_timer_interrupt_blink
 *
 * Objective:
 *
 *   Blink the built-in LED at EXACTLY 1 Hz using a hardware
 *   TIMER INTERRUPT -- no millis(), no delay(). The timer
 *   counts on its own in hardware and fires an interrupt
 *   (ISR) at a precise interval; the ISR toggles the LED.
 *   loop() stays empty to prove the blink runs without the
 *   CPU's help.
 *
 * Background -- what a hardware timer is:
 *
 *   The ATmega328P has counters that tick up on their own,
 *   driven straight off the 16 MHz system clock -- no code
 *   needed to advance them. Timer1 is 16-bit (counts
 *   0..65535). You can ask it to fire an interrupt the
 *   instant its count reaches a target value you choose.
 *
 *   The clock is fast (16,000,000 ticks/sec), so we first
 *   slow it with a PRESCALER (divide by 1/8/64/256/1024),
 *   then count up to a target (OCR1A) in CTC mode ("Clear
 *   Timer on Compare match"): when TCNT1 == OCR1A the timer
 *   resets to 0 and fires the Compare-Match-A interrupt.
 *
 *   The target value is:
 *
 *       OCR1A = F_CPU / (prescaler * target_freq) - 1
 *
 *   For 1 Hz with prescaler 256:
 *
 *       OCR1A = 16,000,000 / (256 * 1) - 1 = 62499
 *
 *   (62499 fits in Timer1's 16 bits, max 65535. The "-1" is
 *   because the timer counts from 0, so 0..62499 = 62500
 *   ticks, and 62500 * (256/16MHz) = 1.000 s exactly.)
 *
 * Registers you'll touch (all pre-named by Arduino.h):
 *
 *   TCCR1A / TCCR1B - Timer1 control (mode + prescaler bits)
 *   TCNT1           - the live 16-bit count
 *   OCR1A           - the compare target
 *   TIMSK1          - timer interrupt enables (OCIE1A bit)
 *   bits: WGM12 (CTC mode), CS12 (prescaler 256),
 *         OCIE1A (compare-match-A interrupt enable)
 *
 * Hardware:
 *
 *   Built-in LED -> Pin 13 (PB5). No wiring needed.
 *   (Optional external LED on pin 13 to see it better.)
 *
 * Behaviour:
 *
 *   The LED toggles once per second -> a steady 1 s ON,
 *   1 s OFF blink, dead-accurate, with an empty loop().
 *
 * Requirements:
 *
 *   - Configure pin 13 (PB5) as OUTPUT via DDRB.
 *   - Set up Timer1 in CTC mode, prescaler 256, with the
 *     Compare-Match-A interrupt enabled, so it fires once
 *     per second.
 *   - Write an ISR that toggles the LED bit on PORTB.
 *   - loop() must stay empty -- the blink is 100% interrupt
 *     driven.
 *
 * Rules:
 *
 *   - No millis(), no delay(), no digitalWrite() in the
 *     blink path. Registers + ISR only.
 *   - Configure the timer with interrupts paused
 *     (cli() ... sei()), then re-enable.
 *   - Do NOT touch Timer0 -- the Arduino core uses it for
 *     millis()/delay(). Use Timer1.
 *
 * Concepts Practiced:
 *
 *   C++:
 *     - ISR(...) interrupt handlers, interrupt vectors
 *     - volatile, cli()/sei() (atomic config)
 *     - Bit setting in control registers (from #25)
 *
 *   Embedded:
 *     - Hardware timers / counters
 *     - Prescalers and the system clock
 *     - CTC mode (Clear Timer on Compare match)
 *     - Compare registers (OCR) and interrupt masks (TIMSK)
 *     - Precise, CPU-free, jitter-free timing
 *
 * Learning Goal:
 *
 *   See that "timing" doesn't have to be a busy loop. A
 *   peripheral can keep perfect time in hardware and tap the
 *   CPU on the shoulder (an interrupt) only at the exact
 *   moment something is due. This is how millis() itself is
 *   built, and how real firmware schedules work.
 *
 * Stretch Goals (optional):
 *
 *   - Change to 2 Hz or 10 Hz by recomputing OCR1A.
 *   - Put a fast counter on pins 8/9/10 in loop() and watch
 *     the 1 Hz blink stay perfectly on time regardless.
 *   - Use a volatile counter incremented in the ISR and
 *     printed from loop() (mind the shared-variable rules).
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Built-in LED on pin 13 = PORTB bit PB5.
const uint8_t LED_BIT = PB5;

// Timer1 compare target for a 1 Hz interrupt at prescaler 256:
//   OCR1A = F_CPU / (prescaler * freq) - 1 = 16e6 / (256 * 1) - 1 = 62499
const uint16_t OCR1A_1HZ = 62499;

void setup() {
  // TODO 1: make pin 13 (PB5) an output via DDRB.

  // TODO 2: configure Timer1 (do this with interrupts paused -- cli() ... sei()):
  //   a) TCCR1A = 0; TCCR1B = 0;  reset TCNT1 = 0     (clean slate)
  //   b) OCR1A  = OCR1A_1HZ;                          (the compare target)
  //   c) select CTC mode      -> set WGM12  bit in TCCR1B
  //   d) select prescaler 256 -> set CS12   bit in TCCR1B
  //   e) enable compare-match-A interrupt -> set OCIE1A bit in TIMSK1
  //   (The notes explain every bit and where OCR1A_1HZ comes from.)
}

// TODO 3: define the interrupt handler. It runs AUTOMATICALLY each time the
//   timer's count reaches OCR1A. Keep it tiny -- just toggle the LED bit:
//
//     ISR(TIMER1_COMPA_vect) {
//       PORTB ^= (1 << LED_BIT);
//     }

void loop() {
  // Intentionally empty: the timer + ISR blink the LED entirely in hardware,
  // independent of loop(). (Stretch: do other work here and the 1 Hz blink
  // stays perfectly on time.)
}
