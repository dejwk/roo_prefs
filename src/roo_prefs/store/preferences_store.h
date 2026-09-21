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
#include "roo_prefs/store/store.h"

namespace roo_prefs {

/// Low-level wrapper around the underlying preferences storage.
class PreferencesStore : public Store {
 public:
  bool isKey(const char* key) override;

  ClearResult clear(const char* key) override;

  template <typename T>
  WriteResult writeObject(const char* key, const T& val) {
    return writeObjectInternal(key, &val, sizeof(val));
  }

  WriteResult writeBool(const char* key, bool val) override;

  WriteResult writeU8(const char* key, uint8_t val) override;

  WriteResult writeI8(const char* key, int8_t val) override;

  WriteResult writeU16(const char* key, uint16_t val) override;

  WriteResult writeI16(const char* key, int16_t val) override;

  WriteResult writeU32(const char* key, uint32_t val) override;

  WriteResult writeI32(const char* key, int32_t val) override;

  WriteResult writeU64(const char* key, uint64_t val) override;

  WriteResult writeI64(const char* key, int64_t val) override;

  WriteResult writeFloat(const char* key, float val) override;

  WriteResult writeDouble(const char* key, double val) override;

  WriteResult writeString(const char* key, roo::string_view val) override;

  /// Stores a raw byte blob.
  ///
  /// Empty blobs are unsupported because the underlying Arduino
  /// `Preferences::putBytes()` API rejects zero-length writes.
  WriteResult writeBytes(const char* key, const void* val,
                         size_t len) override;

  template <typename T>
  ReadResult readObject(const char* key, T& val) {
    return readObjectInternal(key, &val, sizeof(val));
  }

  ReadResult readBool(const char* key, bool& val) override;

  ReadResult readU8(const char* key, uint8_t& val) override;

  ReadResult readI8(const char* key, int8_t& val) override;

  ReadResult readU16(const char* key, uint16_t& val) override;

  ReadResult readI16(const char* key, int16_t& val) override;

  ReadResult readU32(const char* key, uint32_t& val) override;

  ReadResult readI32(const char* key, int32_t& val) override;

  ReadResult readU64(const char* key, uint64_t& val) override;

  ReadResult readI64(const char* key, int64_t& val) override;

  ReadResult readFloat(const char* key, float& val) override;

  ReadResult readDouble(const char* key, double& val) override;

  ReadResult readString(const char* key, std::string& val) override;

  ReadResult readBytes(const char* key, void* val, size_t max_len,
                       size_t* out_len) override;

  ReadResult readBytesLength(const char* key, size_t* out_len) override;

 private:
  friend class Collection;

  BeginResult begin(const char* collection_name, bool read_only) override;
  void end() override;

  EnumerateResult enumerateKeys(const char* collection_name, KeyVisitor visitor,
                                void* context) const override;

  WriteResult writeObjectInternal(const char* key, const void* val,
                                  size_t size) override;

  ReadResult readObjectInternal(const char* key, void* val,
                                size_t size) override;

  Preferences prefs_;
};

}  // namespace roo_prefs

#endif  // ROO_PREFS_USE_ESP32_NVS
