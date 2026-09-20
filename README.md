# roo_prefs

Store configuration options and preferences, such as WiFi passwords, temperature thresholds, alarm timers, etc., persistently, but without hard-coding them in your program, and without using external storage.

Make your libraries configurable, without interfering with other libraries that may also be using persistent storage.

On ESP32, uses the ESP-IDF NVS API directly. The same storage backend works
with both Arduino-ESP32 and native ESP-IDF; Arduino is only needed for optional
`ArduinoString` accessors. On other Arduino boards, the library uses the
platform's `Preferences` library when a compatible implementation is
available.

Written for and tested with the ESP32 family of microcontrollers. It also
supports non-ESP32 Arduino targets that provide a compatible `Preferences`
library, and its storage boundary is small enough to port to other C++
platforms.

For a complete walkthrough, see the [programming guide](doc/programming_guide.md).

Basic usage:

```cpp
// Declared statically.
roo_prefs::Collection prefs("my_lib");

roo_prefs::String wifi_password(prefs, "wifi_pwd");

// Update the preference.
void StoreWiFiPassword(const std::string& password) {
  wifi_password.set(password);
}

// Fetch the stored preference. The value is cached in RAM, so the repetitive
// calls are cheap.
const std::string& GetWiFiPassword() {
  return wifi_password.get();
}
```

## Host emulation

Host builds use the roo_testing 2.0 Arduino ESP32 profile. With Bazelisk 1.21
or newer, a plain command defaults to that profile and prints a notice:

    bazel test ...
    bazel test ... --config=asan
    bazel test ... --config=roo_testing_arduino_esp32

The core tests also run without the Arduino frontend:

    bazel test //:prefs_test //:lazy_write_pref_test \
        --config=roo_testing_idf_esp32

The native ESP-IDF example initializes NVS before accessing its preferences:

    bazel run --config=roo_testing_idf_esp32 //examples/esp_idf/device_settings

The files under .roo_testing/bazelrc/esp32 are vendored from roo_testing;
follow their canonical-source headers when refreshing them.

Arduino examples are native runnable targets in their source packages. For
example:

    bazel run //examples/Basic
