/* Unit tests for the portable soiling core (no Arduino dependency).
 *
 * Build & run (from repo root):
 *   g++ -std=c++11 -Wall -Wextra -o tests/test_soiling_logic.exe tests/test_soiling_logic.cpp
 *   ./tests/test_soiling_logic.exe
 *
 * Case IDs (T1..) map to the test matrix in documentation/05_testing.md.
 * TDD rule: these tests are written BEFORE the implementation they verify.
 */
#include <cstdio>
#include "../firmware/solar_health_monitor/soiling_logic.h"

using namespace solar;

static int g_checks = 0;
static int g_fails = 0;

#define CHECK(cond, msg)                                                     \
  do {                                                                       \
    ++g_checks;                                                              \
    if (!(cond)) { ++g_fails; std::printf("FAIL L%-4d %s\n", __LINE__, msg); } \
  } while (0)

#define CHECK_NEAR(val, exp, tol, msg)                                       \
  do {                                                                       \
    ++g_checks;                                                              \
    double _v = (val), _e = (exp);                                           \
    if (!((_v > _e ? _v - _e : _e - _v) <= (tol))) {                         \
      ++g_fails;                                                             \
      std::printf("FAIL L%-4d %s (got %.3f want %.3f)\n", __LINE__, msg, _v, _e); \
    }                                                                        \
  } while (0)

static Reading raw(float iT, float iR, float lux, float tC) {
  Reading r;
  r.iTestMa = iT;
  r.iRefMa = iR;
  r.lux = lux;
  r.tempC = tC;
  return r;
}

// Feed the engine n times starting at t0, spacing window samples by stepMs.
static Decision fill(AlertEngine& e, float iT, float iR, int n,
                     unsigned long t0, unsigned long stepMs) {
  Reading r = raw(iT, iR, 45000.0f, 40.0f);
  Decision d = e.update(r, t0);
  for (int i = 1; i < n; ++i)
    d = e.update(r, t0 + (unsigned long)i * stepMs);
  return d;
}

/* ---- T1-T5: the math ---------------------------------------------------- */
static void test_math() {
  CHECK_NEAR(computeSoilingPct(500, 500, 1.0f), 0.0f, 0.01f,
             "T1 clean panels = 0%");
  CHECK_NEAR(computeSoilingPct(400, 500, 1.0f), 20.0f, 0.01f,
             "T2 ratio 0.80 = 20%");
  CHECK_NEAR(computeSoilingPct(475, 500, 1.05f), 9.5238f, 0.01f,
             "T3 calibration k compensates panel mismatch");
  CHECK_NEAR(computeSoilingPct(600, 500, 1.0f), -10.0f, 0.01f,
             "T4 clamped (test stronger than reference)");
  CHECK_NEAR(computeSoilingPct(400, 0, 1.0f), 0.0f, 0.01f,
             "T5 guard: zero reference current");
  CHECK_NEAR(computeSoilingPct(400, 500, 0.0f), 0.0f, 0.01f,
             "T5 guard: zero k");
  CHECK_NEAR(computeSoilingPct(0, 500, 1.0f), 100.0f, 0.01f,
             "T5 dead test panel = 100% clamped");
}

/* ---- T6-T11: validity gate + alert state machine ------------------------ */
static void test_alert_states() {
  Config c;
  c.windowIntervalMs = 0;  // tests push one window sample per update
  AlertEngine e(c);

  // T6: low light => nothing is evaluated, state untouched, window untouched
  Reading low = raw(400, 500, 5000, 40);
  Decision d = e.update(low, 0);
  CHECK(!low.valid, "T6 invalid when lux < 20000");
  CHECK_NEAR(low.soilPct, 0.0f, 0.01f, "T6 soil not computed in low light");
  CHECK(d.state == ST_OK, "T6 state stays OK in low light");
  CHECK(!e.windowFull(), "T6 window untouched in low light");

  // T6b: no evaluation when reference current is tiny
  Reading weak = raw(4, 3, 45000, 40);
  d = e.update(weak, 1000);
  CHECK(!weak.valid, "T6b invalid when I_ref <= 5 mA");

  // T7: alert needs a FULL window (14 samples = no, 15th = yes)
  AlertEngine e2(c);
  d = fill(e2, 400, 500, 14, 0, 0);
  CHECK(!e2.windowFull(), "T7 14 samples = window not full");
  CHECK(d.state == ST_OK, "T7 no alert before window is full");
  Reading fifteenth = raw(400, 500, 45000, 40);
  d = e2.update(fifteenth, 0);
  CHECK(e2.windowFull(), "T7 15th sample fills the window");
  CHECK(d.state == ST_ALERT, "T7 alert after full window >15%");
  CHECK(d.alarm, "T7 alarm flag set");
  CHECK_NEAR(d.avgPct, 20.0f, 0.01f, "T7 running mean = 20%");

  // T8: hysteresis - 15 samples at 7% (below 8%) clear the alert
  d = fill(e2, 465, 500, 15, 1000000, 0);
  CHECK(e2.mean() < 8.0f, "T8 mean below clear threshold");
  CHECK(d.state == ST_OK, "T8 alert cleared after mean drops below 8%");
  Reading check = raw(465, 500, 45000, 40);
  d = e2.update(check, 2000000);
  CHECK(d.state == ST_OK, "T8 state stays OK");

  // T9: mid-band (8..15) HOLDS the previous state - no flicker
  AlertEngine e3(c);
  fill(e3, 400, 500, 15, 0, 0);              // alert ON (20%)
  fill(e3, 465, 500, 7, 100000, 0);           // 7 new at 7% -> mean ~13.9%
  Reading mid = raw(465, 500, 45000, 40);
  d = e3.update(mid, 200000);
  CHECK(d.state == ST_ALERT, "T9 mid-band holds ALERT (no flicker)");
  CHECK(e3.mean() > 8.0f && e3.mean() < 15.0f, "T9 mean inside 8-15 band");

  // T10: fault - reference producing, test panel dead
  AlertEngine e4(c);
  Reading fault = raw(0, 400, 40000, 40);
  d = e4.update(fault, 0);
  CHECK(d.state == ST_FAULT, "T10 fault detected");

  // T11: overheat takes priority, then returns to the soiling state
  AlertEngine e5(c);
  fill(e5, 400, 500, 15, 0, 0);              // alert ON
  Reading hot = raw(400, 500, 45000, 70);
  d = e5.update(hot, 100000);
  CHECK(d.state == ST_OVERHEAT, "T11 overheat priority");
  Reading cooled = raw(400, 500, 45000, 40);
  d = e5.update(cooled, 110000);
  CHECK(d.state == ST_ALERT, "T11 returns to ALERT when temperature recovers");
  fill(e5, 500, 500, 15, 200000, 0);          // cleaned
  Reading clean = raw(500, 500, 45000, 40);
  d = e5.update(clean, 300000);
  CHECK(d.state == ST_OK, "T11 alert clears after cleaning");

  // calibration constant is actually used by the engine
  AlertEngine e6(c);
  e6.setK(1.05f);
  Reading cal = raw(475, 500, 45000, 40);
  e6.update(cal, 0);
  CHECK_NEAR(cal.soilPct, 9.5238f, 0.01f, "engine uses k from setK()");
  CHECK_NEAR(e6.k(), 1.05f, 0.0001f, "k() getter");
}

/* ---- T12-T14: time behaviour (staleness, sample interval, reset) -------- */
static void test_time_behaviour() {
  Config c;
  c.windowIntervalMs = 0;
  AlertEngine e(c);

  // T12: stale data (10 min without a valid reading) forgets the evidence
  fill(e, 400, 500, 15, 0, 0);                    // alert ON at t=0
  CHECK(e.windowFull(), "T12 window full before stale");
  Reading night = raw(400, 500, 5000, 40);         // low light for > 10 min
  Decision d = e.update(night, 610000);
  CHECK(d.state == ST_OK, "T12 stale window clears the alert");
  CHECK(!e.windowFull(), "T12 window emptied");
  // and a fresh 5-minute confirmation is required again
  d = fill(e, 400, 500, 14, 700000, 0);
  CHECK(d.state == ST_OK, "T12 14 fresh samples still no alert");
  Reading again = raw(400, 500, 45000, 40);
  d = e.update(again, 700000);
  CHECK(d.state == ST_ALERT, "T12 alert returns after fresh full window");

  // short low-light spell (< 10 min) must NOT clear anything
  AlertEngine e2(c);
  fill(e2, 400, 500, 15, 0, 0);
  Reading shortCloud = raw(400, 500, 5000, 40);
  d = e2.update(shortCloud, 300000);  // 5 min low light
  CHECK(d.state == ST_ALERT, "T12b short cloud keeps the alert");

  // T13: window samples are rate-limited to windowIntervalMs (default 20 s)
  Config cDef;                                     // defaults: 15 x 20 s = 5 min
  AlertEngine fast(cDef);
  d = fill(fast, 400, 500, 15, 0, 0);              // 15 reads inside one instant
  CHECK(!fast.windowFull(), "T13 interval gate: 15 instant reads = 1 sample");
  AlertEngine slow(cDef);
  d = fill(slow, 400, 500, 15, 0, 20000);          // one read every 20 s
  CHECK(slow.windowFull(), "T13 15 samples at 20 s spacing fills window");
  CHECK(d.state == ST_ALERT, "T13 alert after 5 minutes of readings");

  // T14: reset() returns to a clean state
  AlertEngine e3(c);
  fill(e3, 400, 500, 15, 0, 0);
  e3.reset();
  CHECK(!e3.windowFull(), "T14 reset empties window");
  CHECK(e3.mean() == 0.0f, "T14 reset zeroes mean");
}

int main() {
  test_math();
  test_alert_states();
  test_time_behaviour();
  std::printf("%d checks, %d failures -> %s\n", g_checks, g_fails,
              g_fails ? "FAILED" : "PASSED");
  return g_fails ? 1 : 0;
}
