#include <Arduino.h>

#include "roo_prefs/pref.h"

roo_prefs::Collection prefs("main");
roo_prefs::String wifi_password(prefs, "wifi_pwd");

// Update the preference.
void StoreWiFiPassword(const std::string& password) {
  wifi_password.set(password);
}

// Fetch the stored preference. The value is cached in RAM, so the repetitive
// calls are cheap.
const std::string& GetWiFiPassword() { return wifi_password.get(); }

// Comment-out once the password is already stored, to test persistence.
#define STORE

void setup() {
  Serial.begin(115200);
#ifdef STORE
  Serial.println("Storing the WiFi password.");
  StoreWiFiPassword("my_secret_password");
#endif
}

void loop() {
  Serial.print("Stored WiFi password: ");
  Serial.println(GetWiFiPassword().c_str());
  delay(5000);
}
