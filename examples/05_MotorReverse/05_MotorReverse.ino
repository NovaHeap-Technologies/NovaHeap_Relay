/*
  05_MotorReverse: run a motor forward and reverse with two interlocked relays.

  With two relays (one per direction) switching both on at once, or reversing without
  a pause, can short the supply or stress the motor. The interlock makes sure only
  one is ever on, and waits 500 ms between them so the motor can stop first.

  Type f (forward), r (reverse) or s (stop) in the Serial Monitor at 9600 baud.

  Wiring:  forward relay IN -> pin 5,  reverse relay IN -> pin 6

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

// Not named just "forward" and "reverse": ESP8266 and RP2040 boards already have a reverse().
NHRelay motorForward(5, NH_ACTIVE_LOW);
NHRelay motorReverse(6, NH_ACTIVE_LOW);

void setup() {
  Serial.begin(9600);
  motorForward.begin();
  motorReverse.begin();
  motorForward.interlockWith(motorReverse, 500);  // never both on, 500 ms dead time between them
  Serial.println("f = forward, r = reverse, s = stop");
}

void loop() {
  NHRelay::updateAll();  // updates both relays

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'f') motorForward.on();
    if (c == 'r') motorReverse.on();
    if (c == 's') {
      motorForward.off();
      motorReverse.off();
    }
  }

  static bool wasForward = false, wasReverse = false;
  if (motorForward.isOn() != wasForward || motorReverse.isOn() != wasReverse) {
    wasForward = motorForward.isOn();
    wasReverse = motorReverse.isOn();
    Serial.println(wasForward ? "forward" : wasReverse ? "reverse" : "stopped");
  }
}
