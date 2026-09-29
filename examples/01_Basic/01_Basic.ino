/*
  01_Basic: switch one relay on and off every 2 seconds.

  Wiring (relay module):  IN -> pin 7,  VCC -> 5V,  GND -> GND

  Most blue opto-isolated relay modules are active-LOW: the relay clicks on when IN is LOW.
  If your relay is on while this sketch says "off", change NH_ACTIVE_LOW to NH_ACTIVE_HIGH.

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

NHRelay lamp(7, NH_ACTIVE_LOW);

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
