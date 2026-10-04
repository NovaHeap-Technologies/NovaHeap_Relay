/*
  03_TimedPulse: press a button to switch a relay on for 5 seconds, without delay().

  Good for a door lock, a garage opener or a staircase light. If your board has a
  built-in LED, it keeps blinking the whole time to show loop() is never blocked.
  Pressing the button again while the relay is on starts the 5 seconds over, so
  switch bounce doesn't matter.

  Wiring:  push button between pin 2 and GND (uses the internal pull-up)
           relay module IN -> pin 7
           On ESP32 and ESP8266: button on GPIO 14, relay on GPIO 4

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

// The original ESP32 and the ESP8266 wire GPIO 6 to 11 to their flash chip, and GPIO 2
// is a boot pin there (and often the built-in LED), so they use other pins.
// ESP32-S2, S3 and C3 chips aren't affected and keep the usual pins.
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const int BUTTON_PIN = 14;
const int RELAY_PIN = 4;
#else
const int BUTTON_PIN = 2;
const int RELAY_PIN = 7;
#endif
const unsigned long ON_TIME = 5000;

NHRelay doorLock(RELAY_PIN, NH_ACTIVE_LOW);

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
