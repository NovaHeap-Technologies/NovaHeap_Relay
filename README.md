# NovaHeap_Relay

Safe, non-blocking relay control for Arduino, by NovaHeap Technologies.

[![CI](https://github.com/NovaHeap-Technologies/NovaHeap_Relay/actions/workflows/ci.yml/badge.svg)](https://github.com/NovaHeap-Technologies/NovaHeap_Relay/actions/workflows/ci.yml)

Relays look simple, but they catch people out: boards that switch on when the pin is LOW, relays that click on at power-up, motors reversed without a pause, compressors restarted too soon. NovaHeap_Relay handles all of that in a few lines.

- **Active-HIGH or active-LOW, set once.** After that, `on()` always means on.
- **No click at boot.** `begin()` sets the off level before the pin becomes an output.
- **Interlocks.** Only one relay in a group is ever on (motor forward/reverse, fan speeds), with an optional dead time between them.
- **Minimum on and off times.** Protect compressors, pumps and contactors from rapid cycling.
- **Timed pulses without `delay()`.** `onFor(3000)` switches on for 3 seconds while your sketch keeps running.
- **Emergency off.** `forceOff()` ignores the minimum on time.
- **Runs on any board.** It uses only `digitalWrite()` and `millis()`: no dynamic memory, timers or interrupts.

## Install

**Arduino IDE:** Tools → Manage Libraries, search for **NovaHeap Relay**, click Install.

**Manually:** download this repository as a ZIP, then Sketch → Include Library → Add .ZIP Library.

## Quick start

```cpp
#include <NovaHeap_Relay.h>

NHRelay pump(7, NH_ACTIVE_LOW);

void setup() {
  pump.begin();
}

void loop() {
  pump.on();
  delay(2000);
  pump.off();
  delay(2000);
}
```

## Active-HIGH or active-LOW?

| Your board | Usually | Use |
|---|---|---|
| Opto-isolated relay module (blue relays, `IN1`–`IN8` pins, `JD-VCC` jumper) | clicks on when the pin is LOW | `NH_ACTIVE_LOW` |
| Relay shield, or your own transistor driver | clicks on when the pin is HIGH | `NH_ACTIVE_HIGH` (the default) |

Not sure? Run `01_Basic`. If the relay is on while the Serial Monitor says "off", swap the setting.

## Examples

| Example | Shows |
|---|---|
| `01_Basic` | One relay on and off |
| `02_SerialControl` | A 4-relay board controlled from the Serial Monitor, using an array of relays |
| `03_TimedPulse` | A button switches a door lock on for 5 seconds with `onFor()` |
| `04_CompressorProtection` | A thermostat with minimum on and off times |
| `05_MotorReverse` | Forward and reverse relays interlocked with a 500 ms dead time |
| `06_FanSpeedSelector` | Three interlocked relays for fan speed taps |

## Reference

| Call | Does |
|---|---|
| `NHRelay r(pin, NH_ACTIVE_LOW)` | Create a relay. Polarity defaults to `NH_ACTIVE_HIGH` |
| `r.begin()` | Set up the pin, off. Call once in `setup()` |
| `r.on()` / `r.off()` / `r.toggle()` / `r.set(bool)` | Switch |
| `r.onFor(ms)` | On now, off after `ms`. Calling it again restarts the time |
| `r.forceOff()` | Off immediately, ignoring the minimum on time |
| `r.update()` | Apply timing rules. Call every `loop()` |
| `NHRelay::updateAll()` | `update()` for every relay in one call |
| `r.isOn()` | `true` while the relay is energized |
| `r.isWaiting()` | `true` while a request is held back by a timing rule |
| `r.setMinOnTime(ms)` / `r.setMinOffTime(ms)` | Minimum time between switching |
| `a.interlockWith(b, deadTimeMs)` | Put `a` and `b` in one interlock group |
| `r.pin()` | The pin number |

### How the timing rules behave

- Call `update()` (or `NHRelay::updateAll()`) every `loop()` if you use `onFor()`, minimum times or interlock dead time. Without those, `on()` and `off()` act immediately.
- A request that a rule holds back is **kept, not dropped**: it happens as soon as the rule allows, and `isWaiting()` is `true` until then. A newer request replaces it, so `on()` then `off()` inside the minimum on time just keeps the relay on until the time is up, then switches it off.
- The minimum off time counts from `begin()`. After a power cut, a compressor also waits before it starts. That is on purpose.
- Switching on one relay in an interlock group switches the others off first. It energizes only once they are off and the dead time has passed.
- Link as many relays into a group as you need: `a.interlockWith(b); b.interlockWith(c);`. The group uses the longest dead time given.
- Link relays in `setup()`, before switching them.

### Arrays of relays

Relays can't be copied, because each one knows about the others in its group. Use the brace form for arrays:

```cpp
NHRelay relays[] = {{4, NH_ACTIVE_LOW}, {5, NH_ACTIVE_LOW}, {6, NH_ACTIVE_LOW}};
```

## Hardware notes

- **While the board resets and during the bootloader, pins are inputs**, before your code runs. If your relay clicks at power-up, add a 10 kΩ resistor from the relay input to 5 V (active-LOW boards) or to GND (active-HIGH boards).
- **Don't drive a bare relay coil straight from a pin.** Use a relay module, or a transistor with a flyback diode across the coil.
- **Mains voltage can kill.** Use relays rated for the load, fuse the circuit, keep the wiring in an enclosure, and never work on it while it's plugged in.

## Tests

The switching logic has unit tests that run on a PC against a fake Arduino:

```
make -C extras/test
```

GitHub Actions runs them on every push, lints the library, and compiles the examples for AVR, Uno R4, SAMD, ESP32, ESP8266 and RP2040 boards.

## License

NovaHeap Technologies License (MIT terms): free for personal and commercial use, as long as the copyright notice stays in. See [LICENSE](LICENSE).

---

Made by **NovaHeap Technologies**.
