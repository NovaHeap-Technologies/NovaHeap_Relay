# Changelog

## 1.0.2

- `04_CompressorProtection`: read the right temperature on ESP32. Its ADC reads 0 to 4095,
  not 0 to 1023, so the sketch showed several times the real temperature and the
  compressor never switched off. It now uses the factory-calibrated `analogReadMilliVolts()`.

## 1.0.1

- Examples: use safe pins on ESP32 and ESP8266. The examples drove pins 4 to 8, but those
  boards wire GPIO 6 to 11 to their flash chip, so the sketches compiled and then crashed.
  They now use GPIO 4 and 13, plus 25 and 26 on ESP32 or 5 and 12 on ESP8266, and avoid
  the ESP32 pins that glitch or set boot options at power-up (0, 2, 5, 12, 14, 15).
  `03_TimedPulse`'s button moves from GPIO 2, a boot pin that is often the built-in LED,
  to GPIO 14. ESP32-S2, S3 and C3 chips aren't affected and keep the usual pins.
- README: a hardware note on which ESP32 and ESP8266 pins to use.

## 1.0.0

First release.

- `NHRelay` with active-HIGH and active-LOW polarity
- `begin()` sets the off level before the pin becomes an output, so there's no click
- `on()`, `off()`, `toggle()`, `set()`, `onFor()`, `forceOff()`
- Minimum on and off times; held-back requests are kept, not dropped
- Interlock groups of any size, with dead time
- `update()` and `NHRelay::updateAll()`
- Six examples, and unit tests that run on a PC
