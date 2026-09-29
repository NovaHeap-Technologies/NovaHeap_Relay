# Changelog

## 1.0.0

First release.

- `NHRelay` with active-HIGH and active-LOW polarity
- `begin()` sets the off level before the pin becomes an output, so there's no click
- `on()`, `off()`, `toggle()`, `set()`, `onFor()`, `forceOff()`
- Minimum on and off times; held-back requests are kept, not dropped
- Interlock groups of any size, with dead time
- `update()` and `NHRelay::updateAll()`
- Six examples, and unit tests that run on a PC
