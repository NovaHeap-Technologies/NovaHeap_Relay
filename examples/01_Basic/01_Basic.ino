/*
  01_Basic: switch one relay on and off every 2 seconds.

  Wiring (relay module):  IN -> pin 7 (GPIO 4 on ESP32 and ESP8266),  VCC -> 5V,  GND -> GND

  Most blue opto-isolated relay modules are active-LOW: the relay clicks on when IN is LOW.
  If your relay is on while this sketch says "off", change NH_ACTIVE_LOW to NH_ACTIVE_HIGH.

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

// The original ESP32 and the ESP8266 wire GPIO 6 to 11 to their flash chip, so they use
// another pin. ESP32-S2, S3 and C3 chips aren't affected and keep the usual pin.
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const int RELAY_PIN = 4;
#else
const int RELAY_PIN = 7;
#endif

NHRelay lamp(RELAY_PIN, NH_ACTIVE_LOW);

void setup() {
  Serial.begin(9600);
  lamp.begin();  // the pin goes straight to "off": no click at power-up
}

void loop() {
  lamp.on();
  Serial.println("on");
  delay(2000);

  lamp.off();
  Serial.println("off");
  delay(2000);
}
