/*
  03_TimedPulse: press a button to switch a relay on for 5 seconds, without delay().

  Good for a door lock, a garage opener or a staircase light. If your board has a
  built-in LED, it keeps blinking the whole time to show loop() is never blocked.
  Pressing the button again while the relay is on starts the 5 seconds over, so
  switch bounce doesn't matter.

  Wiring:  push button between pin 2 and GND (uses the internal pull-up)
           relay module IN -> pin 7

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

const int BUTTON_PIN = 2;
const unsigned long ON_TIME = 5000;

NHRelay doorLock(7, NH_ACTIVE_LOW);

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#ifdef LED_BUILTIN  // some boards, like generic ESP32 modules, have no built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  doorLock.begin();
}

void loop() {
  doorLock.update();  // needed for onFor() to switch the relay off on time

  if (digitalRead(BUTTON_PIN) == LOW) {
    doorLock.onFor(ON_TIME);
  }

#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, (millis() / 250) % 2 ? HIGH : LOW);
#endif
}
