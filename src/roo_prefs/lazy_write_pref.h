#pragma once

/// Similar to `Pref<T>`, but does not immediately store written data in
/// persistent storage. Instead, writes with a delay, which depends on whether
/// the data stabilized or keeps changing.
///
/// Depends on the "dejwk/roo_scheduler" library.

#include "roo_prefs/pref.h"
#include "roo_scheduler.h"
#include "roo_time.h"

namespace roo_prefs {

template <typename T>
class LazyWritePref {
 public:
  /// Creates a lazy-write preference. The data is flushed to persistent storage
  /// using `scheduler`, after it has been stable for at least
  /// `stable_write_latency` seconds, but no later than
  /// `unstable_write_latency_s` after the last write.
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
  void maybeFlush();

  Pref<T> pref_;
  uint8_t stable_write_latency_s_;
  uint8_t unstable_write_latency_s_;
  T pending_write_;
  roo_scheduler::SingletonTask flusher_;
  uint32_t last_write_ms_;
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

template <typename T>
LazyWritePref<T>::LazyWritePref(Collection& collection,
                                roo_scheduler::Scheduler& scheduler,
                                const char* key, T default_value,
                                uint8_t stable_write_latency_s,
                                uint8_t unstable_write_latency_s)
    : pref_(collection, key, std::move(default_value)),
      stable_write_latency_s_(stable_write_latency_s),
      unstable_write_latency_s_(unstable_write_latency_s),
      flusher_(scheduler, [this]() { maybeFlush(); }) {
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
  if (pending_write_ == value) return true;
  pending_write_ = value;
  last_change_ms_ = roo_time::Uptime::Now().inMillis();
  if (!has_pending_write()) {
    flusher_.scheduleAfter(roo_time::Seconds(stable_write_latency_s_));
  }
  return true;
}

template <typename T>
void LazyWritePref<T>::maybeFlush() {
  uint32_t now = roo_time::Uptime::Now().inMillis();
  uint32_t ms_elapsed_since_last_write_s = (now - last_write_ms_) / 1000;
  uint32_t ms_elapsed_since_last_change_s = (now - last_change_ms_) / 1000;
  if (ms_elapsed_since_last_write_s >= unstable_write_latency_s_ ||
      ms_elapsed_since_last_change_s >= stable_write_latency_s_) {
    if (pref_.set(pending_write_)) {
      last_write_ms_ = now;
      return;
    }
  }
  /// Reschedule the update.
  flusher_.scheduleAfter(roo_time::Seconds(stable_write_latency_s_),
                         roo_scheduler::PRIORITY_BACKGROUND);
}

template <typename T>
bool LazyWritePref<T>::clear() {
  return pref_.clear();
}

}  // namespace roo_prefs
