/*
  06_FanSpeedSelector: pick a fan speed with three interlocked relays.

  Many AC fans have a separate winding tap for each speed. Two taps connected at once
  short part of the winding, so only one relay may ever be on. An interlock group can
  hold any number of relays.

  Type 0 (off), 1 (low), 2 (medium) or 3 (high) in the Serial Monitor at 9600 baud.

  Wiring:  low relay IN -> pin 4,  medium -> pin 5,  high -> pin 6
           ESP32: GPIO 4, 13 and 25.  ESP8266: GPIO 4, 5 and 12.

  WARNING: fan taps carry mains voltage. Use relay modules rated for it, keep the
  wiring in an enclosure, and don't work on it while it's plugged in.

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

// The original ESP32 and the ESP8266 wire GPIO 6 to 11 to their flash chip, and on ESP32
// some other pins glitch at boot, so they use other pins. ESP32-S2, S3 and C3 chips
// aren't affected and keep the usual pins.
#if defined(CONFIG_IDF_TARGET_ESP32)
NHRelay speeds[] = {{4, NH_ACTIVE_LOW}, {13, NH_ACTIVE_LOW}, {25, NH_ACTIVE_LOW}};
#elif defined(ARDUINO_ARCH_ESP8266)
NHRelay speeds[] = {{4, NH_ACTIVE_LOW}, {5, NH_ACTIVE_LOW}, {12, NH_ACTIVE_LOW}};
#else
NHRelay speeds[] = {{4, NH_ACTIVE_LOW}, {5, NH_ACTIVE_LOW}, {6, NH_ACTIVE_LOW}};
#endif
const char *const NAMES[] = {"low", "medium", "high"};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 3; i++) speeds[i].begin();
  speeds[0].interlockWith(speeds[1], 300);
  speeds[1].interlockWith(speeds[2]);  // same group, still 300 ms
  Serial.println("0 = off, 1 = low, 2 = medium, 3 = high");
}

void loop() {
  NHRelay::updateAll();

  if (!Serial.available()) return;
  char c = Serial.read();

  if (c >= '1' && c <= '3') {
    speeds[c - '1'].on();  // the other two switch off first
    Serial.print("speed: ");
    Serial.println(NAMES[c - '1']);
  } else if (c == '0') {
    for (int i = 0; i < 3; i++) speeds[i].off();
    Serial.println("fan off");
  }
}
