#include "roo_prefs/store/preferences_store.h"

namespace roo_prefs {

bool PreferencesStore::begin(const char* collection_name, bool read_only) {
  return prefs_.begin(collection_name, read_only);
}

void PreferencesStore::end() { prefs_.end(); }

bool PreferencesStore::isKey(const char* key) { return prefs_.isKey(key); }

ClearResult PreferencesStore::clear(const char* key) {
  return prefs_.remove(key) ? ClearResult::kOk : ClearResult::kError;
}

WriteResult PreferencesStore::writeBytes(const char* key, const void* val,
                                         size_t len) {
  return (prefs_.putBytes(key, val, len) > 0) ? WriteResult::kOk
                                              : WriteResult::kError;
}

WriteResult PreferencesStore::writeObjectInternal(const char* key,
                                                  const void* val,
                                                  size_t size) {
  return writeBytes(key, val, size);
}

WriteResult PreferencesStore::writeBool(const char* key, bool val) {
  return (prefs_.putBool(key, val) > 0) ? WriteResult::kOk
                                        : WriteResult::kError;
}

WriteResult PreferencesStore::writeU8(const char* key, uint8_t val) {
  return (prefs_.putUChar(key, val) > 0) ? WriteResult::kOk
                                         : WriteResult::kError;
}

WriteResult PreferencesStore::writeI8(const char* key, int8_t val) {
  return (prefs_.putChar(key, val) > 0) ? WriteResult::kOk
                                        : WriteResult::kError;
}

WriteResult PreferencesStore::writeU16(const char* key, uint16_t val) {
  return (prefs_.putUShort(key, val) > 0) ? WriteResult::kOk
                                          : WriteResult::kError;
}

WriteResult PreferencesStore::writeI16(const char* key, int16_t val) {
  return (prefs_.putShort(key, val) > 0) ? WriteResult::kOk
                                         : WriteResult::kError;
}

WriteResult PreferencesStore::writeU32(const char* key, uint32_t val) {
  return (prefs_.putULong(key, val) > 0) ? WriteResult::kOk
                                         : WriteResult::kError;
}

WriteResult PreferencesStore::writeI32(const char* key, int32_t val) {
  return (prefs_.putLong(key, val) > 0) ? WriteResult::kOk
                                        : WriteResult::kError;
}

WriteResult PreferencesStore::writeU64(const char* key, uint64_t val) {
  return (prefs_.putULong64(key, val) > 0) ? WriteResult::kOk
                                           : WriteResult::kError;
}

WriteResult PreferencesStore::writeI64(const char* key, int64_t val) {
  return (prefs_.putLong64(key, val) > 0) ? WriteResult::kOk
                                          : WriteResult::kError;
}

WriteResult PreferencesStore::writeFloat(const char* key, float val) {
  return (prefs_.putFloat(key, val) > 0) ? WriteResult::kOk
                                         : WriteResult::kError;
}

WriteResult PreferencesStore::writeDouble(const char* key, double val) {
  return (prefs_.putDouble(key, val) > 0) ? WriteResult::kOk
                                          : WriteResult::kError;
}

WriteResult PreferencesStore::writeString(const char* key,
                                          roo::string_view val) {
  return (prefs_.putBytes(key, val.data(), val.size()) > 0)
             ? WriteResult::kOk
             : WriteResult::kError;
}

ReadResult PreferencesStore::readObjectInternal(const char* key, void* val,
                                                size_t size) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  if (prefs_.getBytesLength(key) != size) {
    return ReadResult::kWrongType;
  }
  if (prefs_.getBytes(key, val, size) != size) {
    return ReadResult::kError;
  }
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readBool(const char* key, bool& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_U8) return ReadResult::kWrongType;
  bool v1 = prefs_.getBool(key, false);
  bool v2 = prefs_.getBool(key, true);
  if (v1 != v2) {
    return ReadResult::kError;
  }
  val = v1;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readU8(const char* key, uint8_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_U8) return ReadResult::kWrongType;
  constexpr uint8_t magic = 0xDF;
  uint8_t result = prefs_.getUChar(key, magic);
  if (result == magic) {
    constexpr uint8_t magic2 = 0x12;
    if (prefs_.getUChar(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readI8(const char* key, int8_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_I8) return ReadResult::kWrongType;
  constexpr int8_t magic = 0xDF;
  int8_t result = prefs_.getChar(key, magic);
  if (result == magic) {
    constexpr int8_t magic2 = 0x12;
    if (prefs_.getChar(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readU16(const char* key, uint16_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_U16) return ReadResult::kWrongType;
  constexpr uint16_t magic = 0xDFB1;
  uint16_t result = prefs_.getUShort(key, magic);
  if (result == magic) {
    constexpr uint16_t magic2 = 0x1223;
    if (prefs_.getUShort(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readI16(const char* key, int16_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_I16) return ReadResult::kWrongType;
  constexpr int16_t magic = 0xDFB1;
  int16_t result = prefs_.getShort(key, magic);
  if (result == magic) {
    constexpr int16_t magic2 = 0x1223;
    if (prefs_.getShort(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readU32(const char* key, uint32_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_U32) return ReadResult::kWrongType;
  constexpr uint32_t magic = 0xDFB1BEEF;
  uint32_t result = prefs_.getULong(key, magic);
  if (result == magic) {
    constexpr uint32_t magic2 = 0x12345678;
    if (prefs_.getULong(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readI32(const char* key, int32_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_I32) return ReadResult::kWrongType;
  constexpr int32_t magic = 0xDFB1BEEF;
  int32_t result = prefs_.getLong(key, magic);
  if (result == magic) {
    constexpr int32_t magic2 = 0x12345678;
    if (prefs_.getLong(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readU64(const char* key, uint64_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_U64) return ReadResult::kWrongType;
  constexpr uint64_t magic = 0x3E3E1254DFB1BEEFLL;
  uint64_t result = prefs_.getULong64(key, magic);
  if (result == magic) {
    constexpr uint64_t magic2 = 0x1234567812345678LL;
    if (prefs_.getULong64(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readI64(const char* key, int64_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_I64) return ReadResult::kWrongType;
  constexpr int64_t magic = 0x3E3E1254DFB1BEEF;
  int64_t result = prefs_.getLong64(key, magic);
  if (result == magic) {
    constexpr int64_t magic2 = 0x1234567812345678LL;
    if (prefs_.getLong64(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readFloat(const char* key, float& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  constexpr float magic = -0.123456;
  float result = prefs_.getFloat(key, magic);
  if (result == magic) {
    constexpr float magic2 = 1.54e10;
    if (prefs_.getFloat(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readDouble(const char* key, double& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  constexpr double magic = -0.123456;
  double result = prefs_.getDouble(key, magic);
  if (result == magic) {
    constexpr double magic2 = 1.54e10;
    if (prefs_.getDouble(key, magic2) != result) return ReadResult::kError;
  }
  val = result;
  return ReadResult::kOk;
}

ReadResult PreferencesStore::readString(const char* key, std::string& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  size_t size = prefs_.getBytesLength(key);
  if (size == 0) {
    val.clear();
    return ReadResult::kOk;
  }
  auto buf = std::unique_ptr<char[]>(new char[size]);
  if (prefs_.getBytes(key, &buf[0], size) == size) {
    val = std::string(&buf[0], size);
    return ReadResult::kOk;
  }
  return ReadResult::kError;
}

ReadResult PreferencesStore::readBytes(const char* key, void* val,
                                       size_t max_len, size_t* out_len) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  size_t size = prefs_.getBytesLength(key);
  if (out_len != nullptr) *out_len = size;
  if (size == 0) return ReadResult::kOk;
  if (size > max_len) return ReadResult::kError;
  if (prefs_.getBytes(key, val, size) == size) {
    return ReadResult::kOk;
  }
  return ReadResult::kError;
}

ReadResult PreferencesStore::readBytesLength(const char* key, size_t* out_len) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return ReadResult::kNotFound;
  if (type != PT_BLOB) return ReadResult::kWrongType;
  size_t size = prefs_.getBytesLength(key);
  if (size == 0) return ReadResult::kError;
  if (out_len != nullptr) *out_len = size;
  return ReadResult::kOk;
}

}  // namespace roo_prefs