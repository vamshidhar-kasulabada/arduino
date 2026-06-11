# State Machines (Challenge #5)

A companion to `05_reaction_timer.ino`. The reaction-timer game has to *remember what
phase it's in* — waiting to start, counting down, or measuring your reflexes. That memory
is a **state machine**. This note builds the idea from scratch and ties it to the code.

---

## 1. Why you need "state" at all

`loop()` runs over and over, thousands of times a second, and it has no memory of its own.
Yet the game must behave **differently depending on what already happened**:

- Before the first press → ignore everything, wait for a start.
- After start, LED still off → an early press means "Too Early!"
- After the LED turns on → the next press *is* the reaction we measure.

The same input (a button press) means three different things. A single variable records
"where we are," and `loop()` reads it to decide what to do:

```cpp
int gameState = 0;   // 0 = idle, 1 = counting down, 2 = waiting for reaction
```

That variable *is* the state machine. Everything else hangs off it.

---

## 2. What a finite state machine (FSM) is

An FSM is a system that is always in exactly **one** of a fixed set of states, and moves
("transitions") between them only on defined events. Three ingredients:

1. **States** — the distinct modes (`0`, `1`, `2` here).
2. **Transitions** — the rules that change `gameState`.
3. **Events** — what triggers a transition (a button edge, a timer elapsing).

The reaction timer's machine:

```
            press button
   ┌──────────────────────────┐
   v                           |
 [0 IDLE] ──press──> [1 WAIT] ──timer elapsed──> [2 MEASURE]
   ^   ^                 |                            |
   |   |          early press (HIGH->LOW)             |
   |   └─────────────────┘  "Too Early!"             |
   |                                                  |
   └──────────────── press (reaction) ───────────────┘
                     print reaction time
```

Exactly one box is "lit" at any moment. The whole game is "which box, and what gets us to
the next box."

---

## 3. State 0 — IDLE (wait for the player to start)

```cpp
if (gameState == 0) {
  digitalWrite(13, LOW);
  int currentBtnState = digitalRead(8);
  if (prevBtnState == HIGH && currentBtnState == LOW) {   // falling edge = press
    gameState = 1;                                        // TRANSITION 0 -> 1
    gameStartTime = millis();
    time = random(2000, 8000);
    Serial.println("Game Started, wait for led to switch on");
  }
  prevBtnState = currentBtnState;
}
```

The LED is forced off. We watch for a **falling edge** (`prev HIGH && current LOW`), and on
it we:
- switch to state 1,
- stamp `gameStartTime = millis()` (the countdown's start),
- roll a fresh random delay `time` between 2000 and 8000 ms.

### Random numbers

`random(2000, 8000)` returns a value in `[2000, 8000)`. But computers are deterministic —
without a seed you'd get the *same* "random" sequence every power-up. `setup()` fixes that:

```cpp
randomSeed(analogRead(A0));   // seed from electrical noise on a floating pin
```

A0 has nothing connected, so its reading is jittery noise — a cheap source of entropy to
seed the generator differently each run.

---

## 4. State 1 — WAIT (random delay, watch for cheaters)

```cpp
} else if (gameState == 1) {
  if (millis() - gameStartTime >= time) {        // countdown finished?
    digitalWrite(13, HIGH);
    Serial.println("LED ON");
    btnPressedTime = millis();                    // start the reaction clock
    gameState = 2;                                // TRANSITION 1 -> 2
  } else {
    int currentBtnState = digitalRead(8);
    if (prevBtnState == HIGH && currentBtnState == LOW) {
      Serial.println("Too Early!!");
      gameState = 0;                              // TRANSITION 1 -> 0 (penalty)
    }
    prevBtnState = currentBtnState;
  }
}
```

Two competing events race here:

- **Timer wins:** `millis() - gameStartTime >= time` → light the LED, stamp
  `btnPressedTime`, go to state 2. (Same overflow-safe `millis()` idiom from Challenge #4.)
- **Player jumps the gun:** a press *before* the timer elapses → "Too Early!!", reset to
  state 0.

This is the key insight of an FSM: **the same input means different things in different
states.** A press in state 1 is cheating; a press in state 2 is the answer.

---

## 5. State 2 — MEASURE (capture the reaction time)

```cpp
} else if (gameState == 2) {
  int currentBtnState = digitalRead(8);
  if (prevBtnState == HIGH && currentBtnState == LOW) {
    unsigned long reactionTime = millis() - btnPressedTime;   // elapsed since LED on
    Serial.print("Your Reaction Time is: ");
    Serial.println(reactionTime);
    gameState = 0;                                            // TRANSITION 2 -> 0
    digitalWrite(13, LOW);
  }
  prevBtnState = currentBtnState;
}
```

`btnPressedTime` was stamped the instant the LED came on. The first press now subtracts the
two timestamps to get the reaction time in milliseconds, prints it, and returns to IDLE for
the next round.

This is the same **timestamp-and-subtract** measurement pattern as Challenge #4 — here
used to *measure* an interval rather than to *wait out* one.

---

## 6. Edge detection across the whole machine

Notice `prevBtnState` is **one shared variable**, declared once:

```cpp
int prevBtnState = HIGH;
```

Every state that reads the button does the `prev HIGH && current LOW` check and then updates
`prevBtnState = currentBtnState;`. Because the button is wired with `INPUT_PULLUP`, idle =
HIGH and pressed = LOW, so a press is always the **falling edge**. Detecting the edge (not
the level) is what stops one physical press from being counted on every loop the button is
held down.

---

## 7. The `if/else if` ladder vs. a `switch`

This sketch dispatches on state with an `if / else if` chain. The exact same structure is
often written as a `switch` (as Challenge #6 does), and with an `enum` for readable names:

```cpp
enum GameState { IDLE, WAIT, MEASURE };   // instead of magic numbers 0,1,2
GameState gameState = IDLE;

switch (gameState) {
  case IDLE:    /* ... */ break;
  case WAIT:    /* ... */ break;
  case MEASURE: /* ... */ break;
}
```

Using `0`, `1`, `2` works but they're "magic numbers" — an `enum` names each state so the
code reads like the diagram. (That's exactly the upgrade you'll see in the traffic-light
challenge.)

---

## 8. Embedded relevance

- **FSMs are the workhorse of firmware.** Communication protocols, menu systems, button
  debouncers, and motor controllers are nearly always FSMs — they're predictable, testable,
  and use almost no RAM (one `int`).
- **Non-blocking by construction.** Because each state just checks conditions and returns,
  `loop()` never stalls. The MCU can interleave other work between transitions. (Contrast a
  `delay()`-based game, which would be frozen during the random wait and miss early presses.)
- **One state at a time = no contradictions.** You can never be "counting down" and
  "measuring" simultaneously, which eliminates a whole class of bugs.

---

## Gotchas

| Pitfall | Fix |
|---|---|
| Same press counted every loop | Edge-detect (`prev HIGH && current LOW`), update `prevBtnState` each loop |
| Same random sequence every boot | Seed with `randomSeed(analogRead(A0))` from a floating pin |
| `random(2000, 8000)` includes 8000 | No — upper bound is exclusive: range is `[2000, 8000)` |
| `int` for `reactionTime` / timestamps | Use `unsigned long` to match `millis()` and survive overflow |
| Forgetting to reset state after a round | Each terminal action sets `gameState = 0` to re-arm |
| Magic numbers `0/1/2` | Prefer an `enum` so states have names |

---

## TL;DR

A finite state machine gives `loop()` a memory: a single variable (`gameState`) says which
of a fixed set of modes you're in, and transitions move between them on events (a button
edge, a timer elapsing). The reaction timer cycles IDLE → WAIT → MEASURE, where the *same*
button press means "start," "too early," or "that's your time" depending on the current
state. Edge detection isolates real presses, `millis()` keeps it non-blocking, and `enum`
names would make the states read like the diagram.
