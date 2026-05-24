# Programming guide

## When to use this library?

`roo_prefs` is a small wrapper around the Arduino `Preferences` library. Use it
for application or library settings that must survive resets: WiFi credentials,
thresholds, user choices, calibration constants, counters, timers, and similar
small values.

The library adds three conveniences on top of Arduino `Preferences`:

* Namespaced collections, so independent modules can persist data without
  stepping on each other.
* RAII transactions, so `Preferences::begin()` and `Preferences::end()` stay
  paired automatically, including in nested helper code.
* Typed preference objects, which read lazily, cache the value in RAM, and
  expose a small `get()` / `set()` / `clear()` API.


## Getting started

### Basic setup

Include the umbrella header, declare a collection, then declare one or more
typed preferences in that collection:

```cpp
#include "roo_prefs.h"

roo_prefs::Collection prefs("main");

roo_prefs::String wifi_password(prefs, "wifi_pwd");
roo_prefs::Float target_pool_temp(prefs, "pool_temp", 28.0f);
roo_prefs::Bool pump_enabled(prefs, "pump_en", true);

void StoreWiFiPassword(const std::string& password) {
  wifi_password.set(password);
}

const std::string& GetWiFiPassword() {
  return wifi_password.get();
}
```

There is no separate `begin()` call. The first read or write opens the
underlying namespace, performs the operation, and closes it again. The value is
then cached in the `Pref` object, so repeated `get()` calls are cheap.

Prefer declaring collections and preferences statically or globally. The
collection must outlive every preference that refers to it, and collection
names and keys should be stable string literals or other storage with static
lifetime.

### Collections and keys

A `roo_prefs::Collection` corresponds to one Arduino `Preferences` namespace:

```cpp
roo_prefs::Collection network_prefs("network");
roo_prefs::Collection ui_prefs("ui");
```

Use one collection for a related group of settings. If you are writing a
library, choose a collection name that is unique to that library. That lets the
library store its own state without requiring the application to reserve global
keys for it.

Collection names and keys become part of the persistent storage layout.
Changing one later is equivalent to creating a new preference: the old value
remains in storage, but the new name will not find it. If you need to rename a
setting, do it deliberately with a migration step.

On ESP32, the underlying NVS implementation keeps names short. In practice,
keep collection names and keys compact, ASCII-only, and no more than 15
characters.

## Typed preferences

### Common aliases

For common scalar and text types, use the predefined aliases:

```cpp
roo_prefs::Bool flag(prefs, "flag");
roo_prefs::Uint8 counter8(prefs, "cnt8");
roo_prefs::Int16 offset(prefs, "offset");
roo_prefs::Uint32 boot_count(prefs, "boots");
roo_prefs::Int64 large_signed(prefs, "i64");
roo_prefs::Float ratio(prefs, "ratio");
roo_prefs::Double precise(prefs, "precise");
roo_prefs::String label(prefs, "label");

#ifdef ARDUINO
roo_prefs::ArduinoString arduino_label(prefs, "ard_label");
#endif
```

### Defaults and `isSet()`

Every preference has a default value. If you do not pass one, the default is
`T()`:

```cpp
roo_prefs::Int32 retries(prefs, "retries");        // Defaults to 0.
roo_prefs::Int32 timeout_ms(prefs, "timeout", 5);  // Defaults to 5.
```

`get()` returns the stored value when the key exists. If the key does not
exist, it returns the default value. Use `isSet()` when you need to know
whether the current value came from storage or from the built-in default:

```cpp
if (!timeout_ms.isSet()) {
  Serial.println("Using the built-in timeout default.");
}

Serial.println(timeout_ms.get());
```

This distinction is useful for first-run setup, optional calibration, and UI
that should show whether a value has been customized.

Avoid calling `get()`, `isSet()`, `set()`, or `clear()` from constructors.
Preference access depends on the underlying storage backend being initialized
first, so reads and writes should happen during normal setup/runtime code
rather than during object construction, especially for static objects created
before `main()`.

### Writing and clearing

`set()` stores the new value persistently and updates the cached value:

```cpp
CHECK(target_pool_temp.set(29.5f));
```

The return value reports whether the write succeeded. If the cached value is
already equal to the new value, `set()` returns `true` without writing to flash
again.

`clear()` removes the key from persistent storage and resets the cached value
to the default:

```cpp
target_pool_temp.clear();

Serial.println(target_pool_temp.isSet());  // false
Serial.println(target_pool_temp.get());    // 28.0
```

Check the boolean return value of `set()` and `clear()` whenever the setting is
important for correct operation.

### Text preferences

Use `roo_prefs::String` when the stored text value should be exposed as
`std::string`:

```cpp
roo_prefs::Collection prefs("network");
roo_prefs::String wifi_ssid(prefs, "ssid", "pool");

wifi_ssid.set("literal");
wifi_ssid.set(std::string("owned string"));
wifi_ssid.set(roo::string_view("view string"));

const std::string& value = wifi_ssid.get();
```

For `roo_prefs::String`, `set()` accepts `std::string`, string literals,
`const char*`, and `roo::string_view` directly.

On Arduino builds, `roo_prefs::ArduinoString` is also available when you want
the preference to expose Arduino `::String` directly:

```cpp
#ifdef ARDUINO
roo_prefs::ArduinoString wifi_label(prefs, "label", ::String("pool"));
#endif
```

### Transactions

For occasional reads, it is fine to let each preference create its own
implicit transaction. When you are reading or writing several values at once,
create an explicit transaction around them:

```cpp
struct PoolSettings {
  float target_temp;
  bool pump_enabled;
  std::string wifi_password;
};

PoolSettings ReadPoolSettings() {
  roo_prefs::Transaction transaction(prefs, roo_prefs::Transaction::Mode::kReadOnly);
  CHECK(transaction.active());

  return PoolSettings{
      target_pool_temp.get(),
      pump_enabled.get(),
      wifi_password.get(),
  };
}
```

The explicit transaction keeps the namespace open for the whole block. Calls
to `get()` inside the block reuse it through the collection's nested
transaction support.

Use `Transaction(prefs, roo_prefs::Transaction::Mode::kReadOnly)` if you only need
to read the preferences. For writes, you can skip the parameter: `Transaction(prefs)`.

`roo_prefs::Transaction` is a scoped guard around `Preferences::begin()` and
`Preferences::end()`. Transactions are reference-counted and reentrant. If the collection
is already open, inner `get()`, `set()`, and `clear()` calls reuse it automatically. That
makes helper functions easy to compose:

```cpp
roo_prefs::Int32 setting_a(prefs, "a");
roo_prefs::Int32 setting_b(prefs, "b");

void StoreOne(roo_prefs::Int32& pref, int32_t value) {
  roo_prefs::Transaction transaction(prefs);
  CHECK(transaction.active());
  pref.set(value);
}

void StorePair() {
  roo_prefs::Transaction transaction(prefs);
  CHECK(transaction.active());
  StoreOne("a", 1);
  StoreOne("b", 2);
}
```

In the example above, the underlying namespace is opened once by `StorePair()`
and closed when `StorePair()` returns.

Note: while transactions are reentrant, a read-only transaction cannot be
upgraded to read-write. If the collection is already open read-only and inner
code tries to open a write transaction, that inner transaction becomes
inactive. For this reason, when a block may possibly write, make the outermost
transaction read-write from the start.

## Custom types

For small, stable-layout values, you can persist a custom type directly:

```cpp
struct Calibration {
  int32_t offset;
  float scale;

  bool operator==(const Calibration& other) const {
    return offset == other.offset && scale == other.scale;
  }
};

roo_prefs::Pref<Calibration> calibration(
    prefs, "calib", Calibration{0, 1.0f});
```

The object is stored as its binary representation. That keeps the API simple,
but it also means you must be careful:

* Keep custom objects small.
* Do not store pointers, references, or unrelated heap-owning types this way.
* For text, use `roo_prefs::String`, or `roo_prefs::ArduinoString` on Arduino,
  instead of treating text as a generic object blob.
* Low-level blob writes through `Transaction::store().writeBytes()` cannot
  represent empty payloads, because Arduino `Preferences` rejects zero-length
  BLOB writes.
* Keep the struct layout stable across firmware versions, or write an explicit
  migration.
* Provide `operator==`, because `Pref<T>::set()` uses equality to skip
  unnecessary writes.

For anything that may evolve, prefer storing individual fields as separate
typed preferences.

## Deferred writes

Flash storage has a finite write lifetime, and NVS writes are relatively
expensive compared with RAM updates. For settings that may change rapidly, such
as slider positions, counters, or live tuning values, use `LazyWritePref<T>`.

`LazyWritePref<T>` behaves like `Pref<T>` from the caller's point of view, but
`set()` only updates a pending value in RAM. The value is written to persistent
storage later, through a `roo_scheduler::Scheduler` task.

Include the lazy-write header explicitly:

```cpp
#include "roo_prefs/lazy_write_pref.h"

roo_scheduler::Scheduler scheduler;

roo_prefs::Collection prefs("main");
roo_prefs::LazyUint32 brightness(
    prefs, scheduler, "bright", 128,
    2,   // Stable write latency, in seconds.
    10); // Maximum write latency while the value keeps changing.
```

Then make sure the scheduler runs in your application:

```cpp
void loop() {
  scheduler.executeEligibleTasks();

  uint32_t new_brightness = ReadBrightnessSlider();
  brightness.set(new_brightness);
}
```

If the value stops changing, it is flushed after the stable latency. If the
value keeps changing, it is flushed no later than the unstable latency.
`get()` returns the pending value while a write is waiting, so the rest of the
program sees the latest value immediately.

Use plain `Pref<T>` for values that must be persisted immediately before the
program continues, such as credentials accepted from a setup portal. A lazy
preference can lose the most recent update if power is removed before the
scheduled write runs.

## Design patterns

### A small settings module

For an application, a convenient pattern is to keep declarations in one module
and expose intent-focused functions:

```cpp
#include "roo_prefs.h"

namespace settings {

roo_prefs::Collection prefs("pool");

roo_prefs::Float target_temp(prefs, "target", 28.0f);
roo_prefs::Bool auto_mode(prefs, "auto", true);
roo_prefs::String wifi_ssid(prefs, "ssid");
roo_prefs::String wifi_password(prefs, "pwd");

float TargetTemperature() { return target_temp.get(); }

bool SetTargetTemperature(float value) { return target_temp.set(value); }

bool AutoModeEnabled() { return auto_mode.get(); }

bool SetAutoModeEnabled(bool value) { return auto_mode.set(value); }

bool StoreWifi(const std::string& ssid, const std::string& password) {
  roo_prefs::Transaction transaction(prefs);
  if (!transaction.active()) return false;
  return wifi_ssid.set(ssid) && wifi_password.set(password);
}

}  // namespace settings
```

This keeps the storage layout local to the settings module. Other code does not
need to know keys, default values, or whether a setting is stored eagerly or
lazily.

### Library-owned preferences

If you are building a library, avoid requiring the application to pass you raw
`Preferences` objects or shared global keys. Instead, give the library its own
collection name and expose a domain API:

```cpp
class FlowMeterConfig {
 public:
  FlowMeterConfig()
      : prefs_("flow_meter"), pulses_per_liter_(prefs_, "ppl", 450.0f) {}

  float pulses_per_liter() const { return pulses_per_liter_.get(); }

  bool set_pulses_per_liter(float value) {
    return pulses_per_liter_.set(value);
  }

 private:
  roo_prefs::Collection prefs_;
  roo_prefs::Float pulses_per_liter_;
};
```

Declare members in lifetime order: the collection first, then the preferences
that refer to it.

## Practical advice

* Keep values small. Store files, logs, and large state elsewhere.
* Prefer typed preferences for ordinary settings, and lazy preferences for
  values that change frequently.
* Check `set()` and `clear()` return values for important settings.
* Use `isSet()` when the difference between an unset preference and a stored
  default value matters.
* Group multi-value operations in a `Transaction`.
* Be careful not to not write from a read-only transaction.
* Use `CHECK(transaction.active())` when a failed open is unexpected and should
  abort the current flow; branch on `active()` only when the caller can
  recover.
* Use lazy preferences for noisy UI or tuning values, but not for critical
  data that must survive an immediate power loss.
* Use explicit migrations when changing keys, types, or custom struct layouts.
