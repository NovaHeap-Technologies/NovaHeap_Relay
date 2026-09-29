// NovaHeap_Relay: safe, non-blocking relay control for Arduino.
// Copyright (c) 2026 NovaHeap Technologies
// Released under the NovaHeap Technologies License (MIT terms). See LICENSE.

#ifndef NOVAHEAP_RELAY_H
#define NOVAHEAP_RELAY_H

#include <Arduino.h>

// Which pin level switches the relay on.
enum NHPolarity : uint8_t {
  NH_ACTIVE_HIGH,  // pin HIGH = relay on (relay shields, transistor drivers)
  NH_ACTIVE_LOW    // pin LOW = relay on (most opto-isolated relay modules)
};

class NHRelay {
 public:
  NHRelay(uint8_t pin, NHPolarity polarity = NH_ACTIVE_HIGH);
  ~NHRelay();

  // Relays know about each other (interlocks, updateAll), so they can't be copied.
  // For an array, use braces: NHRelay relays[] = {{4, NH_ACTIVE_LOW}, {5, NH_ACTIVE_LOW}};
  NHRelay(const NHRelay &) = delete;
  NHRelay &operator=(const NHRelay &) = delete;

  // Call once in setup(). The pin goes straight to "off", with no click.
  // The minimum off time starts counting here.
  void begin();

  // Switching. A request that a timing rule holds back is kept, not dropped:
  // it happens as soon as the rule allows (call update()). A newer request replaces it.
  void on();
  void off();
  void toggle();
  void set(bool on);
  void onFor(unsigned long ms);  // on now, off after ms; calling again restarts the time
  void forceOff();               // off now, ignoring the minimum on time (emergencies)

  // Call every loop() when using onFor(), minimum times or dead time.
  void update();
  static void updateAll();  // update() for every relay

  bool isOn() const { return _state; }               // relay is energized right now
  bool isWaiting() const { return _want != _state; }  // a request is held back by a timing rule
  uint8_t pin() const { return _pin; }

  // Protect compressors, pumps and contactors from rapid cycling.
  void setMinOnTime(unsigned long ms) { _minOn = ms; }
  void setMinOffTime(unsigned long ms) { _minOff = ms; }

  // Only one relay in an interlock group is ever on. Switching one on switches the
  // others off first, then waits deadTimeMs before energizing. Link as many as needed:
  // a.interlockWith(b); b.interlockWith(c);  The group uses the longest dead time given.
  void interlockWith(NHRelay &other, unsigned long deadTimeMs = 0);

 private:
  void ready();
  void requestOn(unsigned long now);
  void service(unsigned long now);
  bool groupClear(unsigned long now);
  bool inGroup(const NHRelay &other) const;
  void write(bool energize, unsigned long now);

  uint8_t _pin;
  bool _activeLow;
  bool _begun;
  bool _state;   // energized
  bool _want;    // requested
  bool _everOn;
  unsigned long _changedAt;
  unsigned long _minOn;
  unsigned long _minOff;
  unsigned long _deadTime;
  unsigned long _pulseLen;  // 0 = no pulse
  unsigned long _pulseStart;
  NHRelay *_nextInGroup;  // circular list of the interlock group (this, when alone)
  NHRelay *_nextAll;      // list of every relay, for updateAll()
  static NHRelay *_all;
};

#endif
