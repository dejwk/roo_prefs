#pragma once

#include "roo_prefs/store/platform.h"

#if ROO_PREFS_USE_ESP32_NVS

#include "roo_prefs/store/nvs_store.h"

namespace roo_prefs {

using KeyVisitor = bool (*)(void* context, roo::string_view key);

// Backward-compatible name for code that included this low-level store
// directly. ESP32 builds use ESP-IDF NVS and do not require Arduino.
using PreferencesStore = NvsStore;

}  // namespace roo_prefs

#else

#include <memory>

#include "Preferences.h"
#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_prefs/status.h"

namespace roo_prefs {

/// Low-level wrapper around the underlying preferences storage.
class PreferencesStore {
 public:
  bool isKey(const char* key);

  ClearResult clear(const char* key);

  template <typename T>
  WriteResult writeObject(const char* key, const T& val) {
    return writeObjectInternal(key, &val, sizeof(val));
  }

  WriteResult writeBool(const char* key, bool val);

  WriteResult writeU8(const char* key, uint8_t val);

  WriteResult writeI8(const char* key, int8_t val);

  WriteResult writeU16(const char* key, uint16_t val);

  WriteResult writeI16(const char* key, int16_t val);

  WriteResult writeU32(const char* key, uint32_t val);

  WriteResult writeI32(const char* key, int32_t val);

  WriteResult writeU64(const char* key, uint64_t val);

  WriteResult writeI64(const char* key, int64_t val);

  WriteResult writeFloat(const char* key, float val);

  WriteResult writeDouble(const char* key, double val);

  WriteResult writeString(const char* key, roo::string_view val);

  /// Stores a raw byte blob.
  ///
  /// Empty blobs are unsupported because the underlying Arduino
  /// `Preferences::putBytes()` API rejects zero-length writes.
  WriteResult writeBytes(const char* key, const void* val, size_t len);

  template <typename T>
  ReadResult readObject(const char* key, T& val) {
    return readObjectInternal(key, &val, sizeof(val));
  }

  ReadResult readBool(const char* key, bool& val);

  ReadResult readU8(const char* key, uint8_t& val);

  ReadResult readI8(const char* key, int8_t& val);

  ReadResult readU16(const char* key, uint16_t& val);

  ReadResult readI16(const char* key, int16_t& val);

  ReadResult readU32(const char* key, uint32_t& val);

  ReadResult readI32(const char* key, int32_t& val);

  ReadResult readU64(const char* key, uint64_t& val);

  ReadResult readI64(const char* key, int64_t& val);

  ReadResult readFloat(const char* key, float& val);

  ReadResult readDouble(const char* key, double& val);

  ReadResult readString(const char* key, std::string& val);

  ReadResult readBytes(const char* key, void* val, size_t max_len,
                       size_t* out_len);

  ReadResult readBytesLength(const char* key, size_t* out_len);

 private:
  friend class Collection;

  bool begin(const char* collection_name, bool read_only);
  void end();

  EnumerateResult enumerateKeys(const char* collection_name, KeyVisitor visitor,
                                void* context) const;

  WriteResult writeObjectInternal(const char* key, const void* val,
                                  size_t size);

  ReadResult readObjectInternal(const char* key, void* val, size_t size);

  Preferences prefs_;
};

}  // namespace roo_prefs

#endif  // ROO_PREFS_USE_ESP32_NVS
