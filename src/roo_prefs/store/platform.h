#pragma once

// Arduino-ESP32 and native ESP-IDF both expose the ESP-IDF NVS API. Prefer it
// over Arduino's Preferences wrapper whenever the target is an ESP32.
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#define ROO_PREFS_USE_ESP32_NVS 1
#else
#define ROO_PREFS_USE_ESP32_NVS 0
#endif
