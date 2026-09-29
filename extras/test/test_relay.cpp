// Unit tests for NovaHeap_Relay, run on a PC against a fake Arduino.
// Build and run: make -C extras/test

#include <stdio.h>

#include "NovaHeap_Relay.h"

// ---- Fake hardware ---------------------------------------------------------

static const int PINS = 32;
static unsigned long fakeNow;
static uint8_t level[PINS];
static uint8_t mode[PINS];
static uint8_t levelWhenOutput[PINS];  // pin level at the moment it became an output
static bool activeLow[PINS];
static bool exclusive[PINS];  // pins that must never be on together
static bool bothOn;           // set if two exclusive pins were ever on together

unsigned long millis() { return fakeNow; }

void pinMode(uint8_t pin, uint8_t m) {
  if (m == OUTPUT && mode[pin] != OUTPUT) levelWhenOutput[pin] = level[pin];
  mode[pin] = m;
}

static bool pinOn(int pin) { return mode[pin] == OUTPUT && (level[pin] == HIGH) != activeLow[pin]; }

void digitalWrite(uint8_t pin, uint8_t val) {
  level[pin] = val;
  int on = 0;
  for (int p = 0; p < PINS; p++) {
    if (exclusive[p] && pinOn(p)) on++;
  }
  if (on > 1) bothOn = true;
}

static void reset(unsigned long now = 0) {
  fakeNow = now;
  for (int p = 0; p < PINS; p++) {
    level[p] = LOW;
    mode[p] = INPUT;
    levelWhenOutput[p] = 0xFF;
    activeLow[p] = false;
    exclusive[p] = false;
  }
  bothOn = false;
}

static void at(unsigned long now) { fakeNow = now; }

// ---- Test runner -----------------------------------------------------------

static int checks, failures;

#define CHECK(cond)                                                   \
  do {                                                                \
    checks++;                                                         \
    if (!(cond)) {                                                    \
      failures++;                                                     \
      printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);          \
    }                                                                 \
  } while (0)

// ---- Tests -----------------------------------------------------------------

static void activeHigh() {
  reset();
  NHRelay r(2);
  r.begin();
  CHECK(mode[2] == OUTPUT);
  CHECK(levelWhenOutput[2] == LOW);
  CHECK(level[2] == LOW && !r.isOn());
  r.on();
  CHECK(level[2] == HIGH && r.isOn());
  r.off();
  CHECK(level[2] == LOW && !r.isOn());
}

static void activeLowNoGlitchAtBegin() {
  reset();
  activeLow[3] = true;
  NHRelay r(3, NH_ACTIVE_LOW);
  r.begin();
  CHECK(levelWhenOutput[3] == HIGH);  // never drove LOW (= on) while becoming an output
  CHECK(level[3] == HIGH && !r.isOn());
  r.on();
  CHECK(level[3] == LOW);
  r.toggle();
  CHECK(level[3] == HIGH);
  r.set(true);
  CHECK(level[3] == LOW);
  r.set(false);
  CHECK(level[3] == HIGH);
}

static void onWithoutBeginStillWorks() {
  reset();
  NHRelay r(4);
  r.on();
  CHECK(mode[4] == OUTPUT && level[4] == HIGH);
}

static void updateBeforeBeginDoesNothing() {
  reset();
  NHRelay r(5);
  r.update();
  NHRelay::updateAll();
  CHECK(mode[5] == INPUT);
}

static void minOnTimeHoldsOff() {
  reset();
  NHRelay r(6);
  r.setMinOnTime(1000);
  r.begin();
  r.on();
  at(100);
  r.off();
  CHECK(r.isOn() && r.isWaiting());
  at(999);
  r.update();
  CHECK(r.isOn());
  at(1000);
  r.update();
  CHECK(!r.isOn() && !r.isWaiting());
}

static void minOffTimeStartsAtBegin() {
  reset();
  NHRelay r(7);
  r.setMinOffTime(5000);
  r.begin();
  at(10);
  r.on();
  CHECK(!r.isOn() && r.isWaiting());
  at(4999);
  r.update();
  CHECK(!r.isOn());
  at(5000);
  r.update();
  CHECK(r.isOn());
  at(6000);
  r.off();
  at(6001);
  r.on();
  CHECK(!r.isOn());
  at(11000);
  r.update();
  CHECK(r.isOn());
}

static void newRequestReplacesWaitingOne() {
  reset();
  NHRelay r(8);
  r.setMinOnTime(1000);
  r.begin();
  r.on();
  at(100);
  r.off();
  at(200);
  r.on();
  CHECK(r.isOn() && !r.isWaiting());
  at(2000);
  r.update();
  CHECK(r.isOn());
}

static void onForSwitchesOffAfterTime() {
  reset();
  NHRelay r(9);
  r.begin();
  r.onFor(2000);
  CHECK(r.isOn());
  at(1999);
  r.update();
  CHECK(r.isOn());
  at(2000);
  r.update();
  CHECK(!r.isOn());
}

static void onForGivesFullTimeAfterWaiting() {
  reset();
  NHRelay r(10);
  r.setMinOffTime(1000);
  r.begin();
  r.onFor(500);
  CHECK(!r.isOn() && r.isWaiting());
  at(1000);
  r.update();
  CHECK(r.isOn());
  at(1499);
  r.update();
  CHECK(r.isOn());
  at(1500);
  r.update();
  CHECK(!r.isOn());
}

static void onForAgainRestartsTime() {
  reset();
  NHRelay r(11);
  r.begin();
  r.onFor(1000);
  at(800);
  r.onFor(1000);
  at(1799);
  r.update();
  CHECK(r.isOn());
  at(1800);
  r.update();
  CHECK(!r.isOn());
}

static void onCancelsPulse() {
  reset();
  NHRelay r(12);
  r.begin();
  r.onFor(1000);
  at(500);
  r.on();
  at(5000);
  r.update();
  CHECK(r.isOn());
}

static void forceOffIgnoresMinOnTime() {
  reset();
  NHRelay r(13);
  r.setMinOnTime(1000);
  r.setMinOffTime(500);
  r.begin();
  at(500);
  r.on();
  CHECK(r.isOn());
  at(600);
  r.forceOff();
  CHECK(!r.isOn() && !r.isWaiting());
  at(700);
  r.on();
  CHECK(!r.isOn());  // minimum off time counts from the forced off
  at(1100);
  r.update();
  CHECK(r.isOn());
}

static void interlockOnlyOneOn() {
  reset();
  exclusive[14] = exclusive[15] = true;
  NHRelay fwd(14), rev(15);
  fwd.begin();
  rev.begin();
  fwd.interlockWith(rev);
  fwd.on();
  CHECK(fwd.isOn() && !rev.isOn());
  rev.on();
  CHECK(!fwd.isOn() && rev.isOn());
  rev.toggle();
  CHECK(!fwd.isOn() && !rev.isOn());
  CHECK(!bothOn);
}

static void interlockDeadTime() {
  reset();
  exclusive[14] = exclusive[15] = true;
  NHRelay fwd(14), rev(15);
  fwd.begin();
  rev.begin();
  fwd.interlockWith(rev, 500);
  fwd.on();
  CHECK(fwd.isOn());  // no dead time when the other one was never on
  at(100);
  rev.on();
  CHECK(!fwd.isOn() && !rev.isOn() && rev.isWaiting());
  at(599);
  rev.update();
  CHECK(!rev.isOn());
  at(600);
  rev.update();
  CHECK(rev.isOn());
  CHECK(!bothOn);
}

static void interlockWaitsForPartnerMinOnTime() {
  reset();
  exclusive[14] = exclusive[15] = true;
  NHRelay fwd(14), rev(15);
  fwd.setMinOnTime(1000);
  fwd.begin();
  rev.begin();
  fwd.interlockWith(rev);
  fwd.on();
  at(100);
  rev.on();
  CHECK(fwd.isOn() && !rev.isOn() && rev.isWaiting());
  at(999);
  rev.update();
  CHECK(fwd.isOn() && !rev.isOn());
  at(1000);
  rev.update();  // updating only rev must still release fwd
  CHECK(!fwd.isOn() && rev.isOn());
  CHECK(!bothOn);
}

static void groupOfThree() {
  reset();
  exclusive[16] = exclusive[17] = exclusive[18] = true;
  NHRelay low(16), mid(17), high(18);
  low.begin();
  mid.begin();
  high.begin();
  low.interlockWith(mid);
  mid.interlockWith(high);
  low.interlockWith(high);  // already linked: must not split the group
  high.on();
  low.on();
  CHECK(low.isOn() && !mid.isOn() && !high.isOn());
  mid.on();
  CHECK(!low.isOn() && mid.isOn() && !high.isOn());
  high.on();
  CHECK(!low.isOn() && !mid.isOn() && high.isOn());
  CHECK(!bothOn);
}

static void groupUsesLongestDeadTime() {
  reset();
  NHRelay a(19), b(20), c(21);
  a.begin();
  b.begin();
  c.begin();
  a.interlockWith(b, 500);
  a.interlockWith(c);  // no dead time given: 500 stays
  a.on();
  at(100);
  c.on();
  at(599);
  c.update();
  CHECK(!c.isOn());
  at(600);
  c.update();
  CHECK(c.isOn());
}

static void linkingTwoOnRelaysKeepsOne() {
  reset();
  NHRelay a(22), b(23);
  a.on();
  b.on();
  a.interlockWith(b);
  CHECK(a.isOn() && !b.isOn());
}

static void updateAllServicesEveryRelay() {
  reset();
  NHRelay a(24), b(25);
  a.begin();
  b.begin();
  a.onFor(100);
  b.onFor(200);
  at(100);
  NHRelay::updateAll();
  CHECK(!a.isOn() && b.isOn());
  at(200);
  NHRelay::updateAll();
  CHECK(!b.isOn());
}

static void destroyedRelayLeavesListsIntact() {
  reset();
  NHRelay a(26);
  a.begin();
  {
    NHRelay b(27);
    b.begin();
    a.interlockWith(b);
    b.on();
    CHECK(level[27] == HIGH);
  }
  CHECK(level[27] == LOW);  // switched off when destroyed
  a.on();
  CHECK(a.isOn());
  NHRelay c(28);
  c.begin();
  c.onFor(100);
  at(100);
  NHRelay::updateAll();
  CHECK(!c.isOn());
}

static void millisRollover() {
  reset(0xFFFFFF00UL);
  NHRelay r(29);
  r.begin();
  r.onFor(0x200);
  CHECK(r.isOn());
  at(0x000000FFUL);
  r.update();
  CHECK(r.isOn());
  at(0x00000100UL);
  r.update();
  CHECK(!r.isOn());
}

int main() {
  activeHigh();
  activeLowNoGlitchAtBegin();
  onWithoutBeginStillWorks();
  updateBeforeBeginDoesNothing();
  minOnTimeHoldsOff();
  minOffTimeStartsAtBegin();
  newRequestReplacesWaitingOne();
  onForSwitchesOffAfterTime();
  onForGivesFullTimeAfterWaiting();
  onForAgainRestartsTime();
  onCancelsPulse();
  forceOffIgnoresMinOnTime();
  interlockOnlyOneOn();
  interlockDeadTime();
  interlockWaitsForPartnerMinOnTime();
  groupOfThree();
  groupUsesLongestDeadTime();
  linkingTwoOnRelaysKeepsOne();
  updateAllServicesEveryRelay();
  destroyedRelayLeavesListsIntact();
  millisRollover();

  printf("%d checks, %d failed\n", checks, failures);
  return failures ? 1 : 0;
}
