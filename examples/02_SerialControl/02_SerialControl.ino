/*
  02_SerialControl: control a 4-relay board from the Serial Monitor.

  Type 1, 2, 3 or 4 to toggle a relay, 0 to switch them all off.
  Set the Serial Monitor to 9600 baud.

  Wiring: relays on pins 4, 5, 6 and 7 (the pins most 4-relay shields use).
  Relay shields are usually active-HIGH; plug-in relay modules are usually active-LOW.

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

// Relays can't be copied, so an array uses this brace form.
NHRelay relays[] = {{4, NH_ACTIVE_HIGH}, {5, NH_ACTIVE_HIGH}, {6, NH_ACTIVE_HIGH}, {7, NH_ACTIVE_HIGH}};
const int RELAY_COUNT = sizeof(relays) / sizeof(relays[0]);

void printStates() {
  for (int i = 0; i < RELAY_COUNT; i++) {
    Serial.print("Relay ");
    Serial.print(i + 1);
    Serial.print(relays[i].isOn() ? ": ON   " : ": off  ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < RELAY_COUNT; i++) relays[i].begin();
  Serial.println("Type 1-4 to toggle a relay, 0 for all off");
  printStates();
}

void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();

  if (c >= '1' && c < '1' + RELAY_COUNT) {
    relays[c - '1'].toggle();
    printStates();
  } else if (c == '0') {
    for (int i = 0; i < RELAY_COUNT; i++) relays[i].off();
    printStates();
  }
}
