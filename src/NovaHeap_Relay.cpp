// NovaHeap_Relay: safe, non-blocking relay control for Arduino.
// Copyright (c) 2026 NovaHeap Technologies
// Released under the NovaHeap Technologies License (MIT terms). See LICENSE.

#include "NovaHeap_Relay.h"

NHRelay *NHRelay::_all = nullptr;

NHRelay::NHRelay(uint8_t pin, NHPolarity polarity)
    : _pin(pin),
      _activeLow(polarity == NH_ACTIVE_LOW),
      _begun(false),
      _state(false),
      _want(false),
      _everOn(false),
      _changedAt(0),
      _minOn(0),
      _minOff(0),
      _deadTime(0),
      _pulseLen(0),
      _pulseStart(0),
      _nextInGroup(this),
      _nextAll(_all) {
  _all = this;
}

NHRelay::~NHRelay() {
  if (_state) digitalWrite(_pin, _activeLow ? HIGH : LOW);

  NHRelay *prev = this;
  while (prev->_nextInGroup != this) prev = prev->_nextInGroup;
  prev->_nextInGroup = _nextInGroup;

  for (NHRelay **link = &_all; *link; link = &(*link)->_nextAll) {
    if (*link == this) {
      *link = _nextAll;
      break;
    }
  }
}

void NHRelay::begin() {
  // Set the off level before the pin becomes an output. On AVR this latches the level
  // (for active-LOW it turns on the pull-up), so the pin never drives "on", even briefly.
  digitalWrite(_pin, _activeLow ? HIGH : LOW);
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, _activeLow ? HIGH : LOW);
  _begun = true;
  _state = false;
  _want = false;
  _pulseLen = 0;
  _changedAt = millis();
}

void NHRelay::ready() {
  if (!_begun) begin();
}

void NHRelay::on() {
  ready();
  _pulseLen = 0;
  requestOn(millis());
}

void NHRelay::off() {
  ready();
  _pulseLen = 0;
  _want = false;
  service(millis());
}

void NHRelay::toggle() {
  if (_want) {
    off();
  } else {
    on();
  }
}

void NHRelay::set(bool on) {
  if (on) {
    this->on();
  } else {
    off();
  }
}

void NHRelay::onFor(unsigned long ms) {
  if (ms == 0) {
    off();
    return;
  }
  ready();
  unsigned long now = millis();
  _pulseLen = ms;
  _pulseStart = now;  // if the relay is already on, the time counts from now
  requestOn(now);
}

void NHRelay::forceOff() {
  ready();
  _pulseLen = 0;
  _want = false;
  if (_state) write(false, millis());
}

void NHRelay::update() {
  if (_begun) service(millis());
}

void NHRelay::updateAll() {
  unsigned long now = millis();
  for (NHRelay *r = _all; r; r = r->_nextAll) {
    if (r->_begun) r->service(now);
  }
}

void NHRelay::interlockWith(NHRelay &other, unsigned long deadTimeMs) {
  // Swapping the next pointers of two separate circular lists joins them into one.
  if (!inGroup(other)) {
    NHRelay *next = _nextInGroup;
    _nextInGroup = other._nextInGroup;
    other._nextInGroup = next;
  }

  // The whole group uses its longest dead time.
  unsigned long deadTime = deadTimeMs;
  NHRelay *r = this;
  do {
    if (r->_deadTime > deadTime) deadTime = r->_deadTime;
    r = r->_nextInGroup;
  } while (r != this);

  // At most one relay may stay requested on: this one if it is, otherwise the first found.
  NHRelay *keep = _want ? this : nullptr;
  do {
    r->_deadTime = deadTime;
    if (r->_want) {
      if (!keep) {
        keep = r;
      } else if (r != keep) {
        r->_want = false;
        r->_pulseLen = 0;
      }
    }
    r = r->_nextInGroup;
  } while (r != this);

  unsigned long now = millis();
  do {
    if (r->_begun) r->service(now);
    r = r->_nextInGroup;
  } while (r != this);
}

bool NHRelay::inGroup(const NHRelay &other) const {
  const NHRelay *r = this;
  do {
    if (r == &other) return true;
    r = r->_nextInGroup;
  } while (r != this);
  return false;
}

void NHRelay::requestOn(unsigned long now) {
  _want = true;
  for (NHRelay *r = _nextInGroup; r != this; r = r->_nextInGroup) {
    r->_want = false;
    r->_pulseLen = 0;
    r->service(now);
  }
  service(now);
}

void NHRelay::service(unsigned long now) {
  if (_state && _pulseLen && now - _pulseStart >= _pulseLen) {
    _pulseLen = 0;
    _want = false;
  }
  if (_want == _state) return;

  if (_state) {
    if (now - _changedAt >= _minOn) write(false, now);
  } else if (now - _changedAt >= _minOff && groupClear(now)) {
    write(true, now);
  }
}

// True when every other relay in the interlock group is off, and has been off for the
// dead time if it was ever on.
bool NHRelay::groupClear(unsigned long now) {
  for (NHRelay *r = _nextInGroup; r != this; r = r->_nextInGroup) {
    if (r->_state) {
      r->service(now);  // it may be allowed to switch off now
      if (r->_state) return false;
    }
    if (r->_everOn && now - r->_changedAt < _deadTime) return false;
  }
  return true;
}

void NHRelay::write(bool energize, unsigned long now) {
  digitalWrite(_pin, (energize != _activeLow) ? HIGH : LOW);
  _state = energize;
  _changedAt = now;
  if (energize) {
    _everOn = true;
    _pulseStart = now;
  }
}
