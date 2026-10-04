/*
  04_CompressorProtection: a thermostat that never short-cycles the compressor.

  Fridge and AC compressors are damaged when restarted too soon after stopping.
  Minimum on and off times make the relay wait instead, whatever the temperature does.
  The minimum off time also counts from power-up, so after a power cut the compressor
  waits too. That is on purpose.

  Wiring:  LM35 temperature sensor output -> A0 (10 mV per degree C)
           relay module IN -> pin 8 (GPIO 4 on ESP32 and ESP8266)

  The times here are short so you can watch it work. Real compressors usually need
  about 3 minutes off (180000 ms).

  NovaHeap_Relay by NovaHeap Technologies
*/

#include <NovaHeap_Relay.h>

const int SENSOR_PIN = A0;
const float TURN_ON_ABOVE = 26.0;   // degrees C
const float TURN_OFF_BELOW = 24.0;  // the gap stops it chattering around one value

#if defined(ARDUINO_ARCH_ESP32)
// ESP32 reads 0 to 4095, not 0 to 1023, and isn't linear, so use its calibrated millivolts.
float readCelsius() { return analogReadMilliVolts(SENSOR_PIN) / 10.0; }
#else
const float VOLTS_PER_STEP = 5.0 / 1023.0;  // change 5.0 to 3.3 on a 3.3 V board (3.2 on ESP8266)
float readCelsius() { return analogRead(SENSOR_PIN) * VOLTS_PER_STEP * 100.0; }
#endif

// The original ESP32 and the ESP8266 wire GPIO 6 to 11 to their flash chip, so they use
// another pin. ESP32-S2, S3 and C3 chips aren't affected and keep the usual pin.
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const int RELAY_PIN = 4;
#else
const int RELAY_PIN = 8;
#endif

NHRelay compressor(RELAY_PIN, NH_ACTIVE_LOW);

void setup() {
  Serial.begin(9600);
  compressor.setMinOnTime(20000);   // once started, run at least 20 s
  compressor.setMinOffTime(30000);  // once stopped, rest at least 30 s
  compressor.begin();
}

void loop() {
  compressor.update();

  float celsius = readCelsius();
  if (celsius > TURN_ON_ABOVE) {
    compressor.on();
  } else if (celsius < TURN_OFF_BELOW) {
    compressor.off();
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.print(celsius, 1);
    Serial.print(" C  compressor ");
    Serial.print(compressor.isOn() ? "ON" : "off");
    Serial.println(compressor.isWaiting() ? "  (waiting: protecting compressor)" : "");
  }
}
