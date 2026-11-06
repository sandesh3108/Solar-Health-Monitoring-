#pragma once
// ============================================================================
// soiling_logic.h - portable core logic for the Solar Health Monitor
//
// NO Arduino includes: the exact same code is compiled by
//   * the ESP32 firmware (firmware/solar_health_monitor/solar_health_monitor.ino)
//   * the PC unit tests (tests/test_soiling_logic.cpp)
// so the tested behaviour is the behaviour that runs on the board.
//
// Contains: soiling math, validity gate, 5-minute rolling window,
//           hysteresis alert FSM (fault > overheat > soiling priority).
// ============================================================================

namespace solar {

struct Config {
  float luxMin = 20000.0f;                  // below this light: do not evaluate
  float soilAlertOn = 15.0f;                // %  alert ON  threshold
  float soilAlertOff = 8.0f;                // %  alert OFF threshold (hysteresis)
  float tempWarnC = 65.0f;                  // overheat warning
  float iRefMinMa = 5.0f;                   // min reference current to trust ratio
  float iRefFaultMa = 50.0f;                // reference healthy ...
  float iTestFaultMa = 5.0f;                // ... but test panel dead => FAULT
  int windowSize = 15;                      // window samples (<= MAX_WINDOW)
  unsigned long windowIntervalMs = 20000UL; // one window sample / 20 s => 5 min
  unsigned long staleMs = 600000UL;         // forget window after 10 min invalid
};

struct Reading {
  float iTestMa = 0, iRefMa = 0, lux = 0, tempC = 0;  // inputs
  bool valid = false;                                 // output: passed the gate
  float soilPct = 0;                                  // output: soiling %
};

enum State { ST_OK = 0, ST_ALERT = 1, ST_FAULT = 2, ST_OVERHEAT = 3 };

struct Decision {
  State state;
  bool alarm;    // state != ST_OK
  float avgPct;  // rolling mean of the window
};

// Soiling % = (1 - (I_test/I_ref)/k) * 100, clamped to [-10, 100].
inline float computeSoilingPct(float iTestMa, float iRefMa, float k) {
  if (iRefMa <= 0.0f || k <= 0.0f) return 0.0f;  // guard: never divide by zero
  float s = (1.0f - (iTestMa / iRefMa) / k) * 100.0f;
  if (s < -10.0f) s = -10.0f;
  if (s > 100.0f) s = 100.0f;
  return s;
}

class AlertEngine {
 public:
  static const int MAX_WINDOW = 32;

  explicit AlertEngine(const Config& cfg)
      : cfg_(cfg),
        size_(cfg.windowSize > 0 && cfg.windowSize <= MAX_WINDOW ? cfg.windowSize : 15) {}

  void setK(float k) { if (k > 0.0f) k_ = k; }
  float k() const { return k_; }

  void reset() {
    idx_ = 0; filled_ = 0;
    soilAlert_ = false; haveValid_ = false; havePush_ = false;
    lastValidMs_ = 0; lastPushMs_ = 0;
  }

  // One evaluation step. Fills r.valid / r.soilPct and returns the decision.
  Decision update(Reading& r, unsigned long nowMs) {
    // 1. validity gate - no trustworthy ratio, no evaluation, no state change
    r.valid = (r.lux > cfg_.luxMin) && (r.iRefMa > cfg_.iRefMinMa);

    if (r.valid) {
      haveValid_ = true;
      lastValidMs_ = nowMs;
      r.soilPct = computeSoilingPct(r.iTestMa, r.iRefMa, k_);
      if (!havePush_ || nowMs - lastPushMs_ >= cfg_.windowIntervalMs) {
        push(r.soilPct);
        havePush_ = true;
        lastPushMs_ = nowMs;
      }
      float m = mean();
      if (filled_ >= size_ && m > cfg_.soilAlertOn) soilAlert_ = true;  // ON
      if (m < cfg_.soilAlertOff) soilAlert_ = false;                    // OFF
    } else {
      r.soilPct = 0.0f;
      // 10 min without a valid reading => forget stale evidence (e.g. night)
      if (haveValid_ && nowMs - lastValidMs_ > cfg_.staleMs) reset();
    }

    // 2. state priority: fault > overheat > soiling alert
    Decision d;
    if (r.iRefMa > cfg_.iRefFaultMa && r.iTestMa < cfg_.iTestFaultMa)
      d.state = ST_FAULT;
    else if (r.tempC > cfg_.tempWarnC)
      d.state = ST_OVERHEAT;
    else
      d.state = soilAlert_ ? ST_ALERT : ST_OK;
    d.alarm = (d.state != ST_OK);
    d.avgPct = mean();
    return d;
  }

  float mean() const {
    if (filled_ == 0) return 0.0f;
    float s = 0;
    for (int i = 0; i < filled_; ++i) s += buf_[i];
    return s / filled_;
  }
  bool windowFull() const { return filled_ >= size_; }
  int windowFilled() const { return filled_; }

 private:
  void push(float v) {
    buf_[idx_] = v;
    idx_ = (idx_ + 1) % size_;
    if (filled_ < size_) ++filled_;
  }

  Config cfg_;
  float k_ = 1.0f;
  int size_;
  float buf_[MAX_WINDOW] = {};
  int idx_ = 0, filled_ = 0;
  bool soilAlert_ = false, haveValid_ = false, havePush_ = false;
  unsigned long lastValidMs_ = 0, lastPushMs_ = 0;
};

}  // namespace solar
