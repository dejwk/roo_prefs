#pragma once

#include <memory>

#include "Preferences.h"
#include "roo_backport.h"
#include "roo_prefs/status.h"

namespace roo_prefs {

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

 private:
  friend class Collection;

  bool begin(const char* collection_name, bool read_only);
  void end();

  WriteResult writeObjectInternal(const char* key, const void* val,
                                  size_t size);

  ReadResult readObjectInternal(const char* key, void* val, size_t size);

  Preferences prefs_;
};

}  // namespace roo_prefs