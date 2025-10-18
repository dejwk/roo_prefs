# roo_prefs

Store configuration options and preferences, such as WiFi passwords, temperature thresholds, alarm timers, etc., persistently, but without hard-coding them in your program, and without using external storage.

Make your libraries configurable, without interfering with other libraries that may also be using persistent storage.

Uses Arduino Preferences lib under the hood.

Written for and tested with ESP32 family of microcontrollers, but should be easily portable to anything that uses standard C++.

Basic usage:

```cpp
// Declared statically.
roo_prefs::Collection prefs("my_lib");

roo_prefs::String wifi_password(prefs, "wifi_pwd");

// Update the preference.
void StoreWiFiPassword(const std::string& password) {
  wifi_password.set(password);
}

// Fetch the stored preference. The value is cached in RAM, so the repetitive calls are cheap.
const std::string& GetWiFiPassword() {
  return wifi_password.get();
}
```
