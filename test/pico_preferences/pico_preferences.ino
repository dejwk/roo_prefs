#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>

#include <array>
#include <cstring>

#include "roo_prefs.h"

// Run on Arduino-Pico with the LittleFS Preferences library. Send 'w' to
// write fixtures, 'x' to reboot, and 'v' to verify them in a fresh boot.
namespace {

constexpr char kNamespace[] = "roo_prefs_test";
constexpr uint32_t kMarker = 0xdfb1beef;
const std::string kString("a\0b", 3);
constexpr std::array<uint8_t, 3> kBlob = {0x00, 0x7f, 0xff};
bool wrote_this_boot = false;

struct Sample {
  uint16_t first;
  uint16_t second;
};

constexpr Sample kSample = {720, 1024};

// Seeds legacy raw bytes and exercises empty-string writes through roo_prefs.
bool WriteFixtures() {
  Preferences legacy;
  if (!legacy.begin(kNamespace)) return false;
  const bool seeded =
      (!legacy.isKey("empty") || legacy.remove("empty")) &&
      legacy.putULong("marker", kMarker) == sizeof(kMarker) &&
      legacy.putBytes("string", kString.data(), kString.size()) ==
          kString.size() &&
      legacy.putBytes("blob", kBlob.data(), kBlob.size()) == kBlob.size() &&
      legacy.putBytes("sample", &kSample, sizeof(kSample)) == sizeof(kSample) &&
      legacy.putBool("bool", true) == 1 &&
      legacy.putUChar("bad_bool", 2) == 1 &&
      legacy.putInt("signed", -1024) == sizeof(int32_t) &&
      legacy.putFloat("float", 2.5f) == sizeof(float);
  legacy.end();

  roo_prefs::Collection collection(kNamespace);
  roo_prefs::String empty(collection, "empty");
  roo_prefs::String replaced(collection, "replaced");
  if (!(seeded && !empty.isSet() && empty.set("") && replaced.set("nonempty") &&
        replaced.set(""))) {
    return false;
  }
  roo_prefs::Transaction tx(collection);
  return tx.active() &&
         tx.store().writeBytes("empty_blob", kBlob.data(), 0) ==
             roo_prefs::WriteResult::kError &&
         !tx.store().isKey("empty_blob");
}

// Checks status and output for stored, missing, malformed and empty entries.
bool VerifyStore(roo_prefs::Collection& collection) {
  using roo_prefs::ReadResult;
  roo_prefs::Transaction tx(collection,
                            roo_prefs::Transaction::Mode::kReadOnly);
  if (!tx.active()) return false;
  roo_prefs::Store& store = tx.store();
  uint16_t wrong_width = 7;
  uint32_t missing = 12;
  bool invalid_bool = true;
  Sample sample{};
  std::array<uint8_t, kBlob.size()> blob{};
  size_t length = 99;
  return store.readU16("marker", wrong_width) == ReadResult::kWrongType &&
         wrong_width == 7 &&
         store.readU32("missing", missing) == ReadResult::kNotFound &&
         missing == 12 &&
         store.readBool("bad_bool", invalid_bool) == ReadResult::kError &&
         invalid_bool &&
         store.readObject("sample", sample) == ReadResult::kOk &&
         sample.first == kSample.first && sample.second == kSample.second &&
         store.readBytesLength("blob", &length) == ReadResult::kOk &&
         length == blob.size() &&
         store.readBytes("blob", blob.data(), blob.size(), &length) ==
             ReadResult::kOk &&
         length == blob.size() && blob == kBlob &&
         store.readBytesLength("empty", &length) == ReadResult::kOk &&
         length == 0;
}

// Reopens the default backend and reads public preferences after reboot.
bool VerifyFixtures() {
  if (wrote_this_boot) return false;
  roo_prefs::Collection collection(kNamespace);
  roo_prefs::Uint32 marker(collection, "marker");
  roo_prefs::String string(collection, "string");
  roo_prefs::String empty(collection, "empty", "fallback");
  roo_prefs::ArduinoString replaced(collection, "replaced");
  roo_prefs::Bool flag(collection, "bool");
  roo_prefs::Int32 signed_value(collection, "signed");
  roo_prefs::Float float_value(collection, "float");

  if (!(marker.isSet() && marker.get() == kMarker && string.isSet() &&
        string.get() == kString && empty.isSet() && empty.get().empty() &&
        replaced.isSet() && replaced.get().length() == 0 && flag.isSet() &&
        flag.get() && signed_value.isSet() && signed_value.get() == -1024 &&
        float_value.isSet() && float_value.get() == 2.5f &&
        VerifyStore(collection) && marker.clear())) {
    return false;
  }
  roo_prefs::Uint32 cleared(collection, "marker", 42);
  return !cleared.isSet() && cleared.get() == 42;
}

void Result(const char* phase, bool passed) {
  Serial.printf("RESULT %s %s\n", phase, passed ? "PASS" : "FAIL");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  LittleFS.setConfig(LittleFSConfig(false));
}

void loop() {
  switch (Serial.read()) {
    case '?':
      Serial.println("ROO_PREFS_PICO_TEST_READY");
      break;
#if defined(ROO_PREFS_PICO_TEST_FORMAT)
    case 'f':
      Result("format", LittleFS.format());
      break;
#endif
    case 'w':
      wrote_this_boot = true;
      Result("write", WriteFixtures());
      break;
    case 'v':
      Result("verify", VerifyFixtures());
      break;
    case 'x':
      Serial.println("REBOOT");
      Serial.flush();
      delay(50);
      rp2040.reboot();
      break;
    default:
      delay(2);
      break;
  }
}
