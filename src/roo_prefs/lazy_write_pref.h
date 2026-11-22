#pragma once

/// Similar to `Pref<T>`, but does not immediately store written data in
/// persistent storage. Instead, writes with a delay, which depends on whether
/// the data stabilized or keeps changing.
///
/// Depends on the "dejwk/roo_scheduler" library.

#include <algorithm>

#include "roo_prefs/pref.h"
#include "roo_scheduler.h"
#include "roo_time.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace roo_prefs {

template <typename T>
class LazyWritePref {
 public:
  /// Creates a preference that uses @p scheduler to defer writes. A flush is
  /// due @p stable_write_latency_s seconds after the latest value change or
  /// @p unstable_write_latency_s seconds after the first pending change,
  /// whichever comes first. The maximum delay is at least the stable interval.
  LazyWritePref(Collection& collection, roo_scheduler::Scheduler& scheduler,
                const char* key, T default_value = T(),
                uint8_t stable_write_latency_s = 2,
                uint8_t unstable_write_latency_s = 10);

  bool isSet() const;

  const T& get() const;

  bool set(const T& value);

  bool clear();

 private:
  bool has_pending_write() const { return flusher_.is_scheduled(); }

  /// Writes a due value or schedules the next attempt.
  void maybeFlush();

  Pref<T> pref_;
  uint8_t stable_write_latency_s_;
  uint8_t unstable_write_latency_s_;
  T pending_write_;
  roo_scheduler::SingletonTask flusher_;
  uint32_t first_pending_write_ms_;
  uint32_t last_change_ms_;
};

using LazyBool = LazyWritePref<bool>;
using LazyUint8 = LazyWritePref<uint8_t>;
using LazyInt8 = LazyWritePref<int8_t>;
using LazyUint16 = LazyWritePref<uint16_t>;
using LazyInt16 = LazyWritePref<int16_t>;
using LazyUint32 = LazyWritePref<uint32_t>;
using LazyInt32 = LazyWritePref<int32_t>;
using LazyUint64 = LazyWritePref<uint64_t>;
using LazyInt64 = LazyWritePref<int64_t>;
using LazyFloat = LazyWritePref<float>;
using LazyDouble = LazyWritePref<double>;
using LazyString = LazyWritePref<std::string>;

#ifdef ARDUINO
using LazyArduinoString = LazyWritePref<::String>;
#endif

template <typename T>
LazyWritePref<T>::LazyWritePref(Collection& collection,
                                roo_scheduler::Scheduler& scheduler,
                                const char* key, T default_value,
                                uint8_t stable_write_latency_s,
                                uint8_t unstable_write_latency_s)
    : pref_(collection, key, std::move(default_value)),
      stable_write_latency_s_(stable_write_latency_s),
      unstable_write_latency_s_(unstable_write_latency_s),
      pending_write_(),
      flusher_(scheduler, [this]() { maybeFlush(); }),
      first_pending_write_ms_(0),
      last_change_ms_(0) {
  if (unstable_write_latency_s_ < stable_write_latency_s_) {
    unstable_write_latency_s_ = stable_write_latency_s_;
  }
}

template <typename T>
bool LazyWritePref<T>::isSet() const {
  return has_pending_write() || pref_.isSet();
}

template <typename T>
const T& LazyWritePref<T>::get() const {
  return has_pending_write() ? pending_write_ : pref_.get();
}

template <typename T>
bool LazyWritePref<T>::set(const T& value) {
  const bool already_pending = has_pending_write();
  if (already_pending) {
    if (pending_write_ == value) return true;
  } else if (pref_.isSet() && pref_.get() == value) {
    return true;
  }

  const roo_time::Uptime now = roo_time::Uptime::Now();
  pending_write_ = value;
  last_change_ms_ = now.inMillis();
  if (!already_pending) {
    first_pending_write_ms_ = last_change_ms_;
    flusher_.scheduleOn(now + roo_time::Seconds(stable_write_latency_s_));
  }
  return true;
}

template <typename T>
void LazyWritePref<T>::maybeFlush() {
  const roo_time::Uptime now = roo_time::Uptime::Now();
  const uint32_t now_ms = now.inMillis();
  const uint32_t since_first_write_ms = now_ms - first_pending_write_ms_;
  const uint32_t since_change_ms = now_ms - last_change_ms_;
  const uint32_t stable_ms = uint32_t{stable_write_latency_s_} * 1000;
  const uint32_t unstable_ms = uint32_t{unstable_write_latency_s_} * 1000;
  if (since_change_ms < stable_ms && since_first_write_ms < unstable_ms) {
    const uint32_t delay_ms = std::min(stable_ms - since_change_ms,
                                       unstable_ms - since_first_write_ms);
    flusher_.scheduleOn(now + roo_time::Millis(delay_ms),
                        roo_scheduler::Priority::kBackground);
  } else if (!pref_.set(pending_write_)) {
    // Wait between retries even when the initial write was due immediately.
    const uint8_t retry_delay_s = std::max(stable_write_latency_s_, uint8_t{1});
    flusher_.scheduleAfter(roo_time::Seconds(retry_delay_s),
                           roo_scheduler::Priority::kBackground);
  }
}

template <typename T>
bool LazyWritePref<T>::clear() {
  if (!pref_.clear()) return false;
  flusher_.cancel();
  pending_write_ = pref_.get();
  return true;
}

}  // namespace roo_prefs
