#include "roo_prefs/store/preferences_store.h"

namespace roo_prefs {

bool PreferencesStore::begin(const char* collection_name, bool read_only) {
  return prefs_.begin(collection_name, read_only);
}

void PreferencesStore::end() { prefs_.end(); }

bool PreferencesStore::isKey(const char* key) { return prefs_.isKey(key); }

ClearResult PreferencesStore::clear(const char* key) {
  return prefs_.remove(key) ? CLEAR_OK : CLEAR_ERROR;
}

WriteResult PreferencesStore::writeBytes(const char* key, const void* val,
                                         size_t len) {
  return (prefs_.putBytes(key, val, len) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeObjectInternal(const char* key,
                                                  const void* val,
                                                  size_t size) {
  return writeBytes(key, val, size);
}

WriteResult PreferencesStore::writeBool(const char* key, bool val) {
  return (prefs_.putBool(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeU8(const char* key, uint8_t val) {
  return (prefs_.putUChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeI8(const char* key, int8_t val) {
  return (prefs_.putChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeU16(const char* key, uint16_t val) {
  return (prefs_.putUShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeI16(const char* key, int16_t val) {
  return (prefs_.putShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeU32(const char* key, uint32_t val) {
  return (prefs_.putULong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeI32(const char* key, int32_t val) {
  return (prefs_.putLong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeU64(const char* key, uint64_t val) {
  return (prefs_.putULong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeI64(const char* key, int64_t val) {
  return (prefs_.putLong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeFloat(const char* key, float val) {
  return (prefs_.putFloat(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeDouble(const char* key, double val) {
  return (prefs_.putDouble(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

WriteResult PreferencesStore::writeString(const char* key,
                                          roo::string_view val) {
  return (prefs_.putBytes(key, val.data(), val.size()) > 0) ? WRITE_OK
                                                            : WRITE_ERROR;
}

ReadResult PreferencesStore::readObjectInternal(const char* key, void* val,
                                                size_t size) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  if (prefs_.getBytesLength(key) != size) {
    return READ_WRONG_TYPE;
  }
  if (prefs_.getBytes(key, val, size) != size) {
    return READ_ERROR;
  }
  return READ_OK;
}

ReadResult PreferencesStore::readBool(const char* key, bool& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_U8) return READ_WRONG_TYPE;
  bool v1 = prefs_.getBool(key, false);
  bool v2 = prefs_.getBool(key, true);
  if (v1 != v2) {
    return READ_ERROR;
  }
  val = v1;
  return READ_OK;
}

ReadResult PreferencesStore::readU8(const char* key, uint8_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_U8) return READ_WRONG_TYPE;
  constexpr uint8_t magic = 0xDF;
  uint8_t result = prefs_.getUChar(key, magic);
  if (result == magic) {
    constexpr uint8_t magic2 = 0x12;
    if (prefs_.getUChar(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readI8(const char* key, int8_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_I8) return READ_WRONG_TYPE;
  constexpr int8_t magic = 0xDF;
  int8_t result = prefs_.getChar(key, magic);
  if (result == magic) {
    constexpr int8_t magic2 = 0x12;
    if (prefs_.getChar(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readU16(const char* key, uint16_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_U16) return READ_WRONG_TYPE;
  constexpr uint16_t magic = 0xDFB1;
  uint16_t result = prefs_.getUShort(key, magic);
  if (result == magic) {
    constexpr uint16_t magic2 = 0x1223;
    if (prefs_.getUShort(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readI16(const char* key, int16_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_I16) return READ_WRONG_TYPE;
  constexpr int16_t magic = 0xDFB1;
  int16_t result = prefs_.getShort(key, magic);
  if (result == magic) {
    constexpr int16_t magic2 = 0x1223;
    if (prefs_.getShort(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readU32(const char* key, uint32_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_U32) return READ_WRONG_TYPE;
  constexpr uint32_t magic = 0xDFB1BEEF;
  uint32_t result = prefs_.getULong(key, magic);
  if (result == magic) {
    constexpr uint32_t magic2 = 0x12345678;
    if (prefs_.getULong(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readI32(const char* key, int32_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_I32) return READ_WRONG_TYPE;
  constexpr int32_t magic = 0xDFB1BEEF;
  int32_t result = prefs_.getLong(key, magic);
  if (result == magic) {
    constexpr int32_t magic2 = 0x12345678;
    if (prefs_.getLong(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readU64(const char* key, uint64_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_U64) return READ_WRONG_TYPE;
  constexpr uint64_t magic = 0x3E3E1254DFB1BEEFLL;
  uint64_t result = prefs_.getULong64(key, magic);
  if (result == magic) {
    constexpr uint64_t magic2 = 0x1234567812345678LL;
    if (prefs_.getULong64(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readI64(const char* key, int64_t& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_I64) return READ_WRONG_TYPE;
  constexpr int64_t magic = 0x3E3E1254DFB1BEEF;
  int64_t result = prefs_.getLong64(key, magic);
  if (result == magic) {
    constexpr int64_t magic2 = 0x1234567812345678LL;
    if (prefs_.getLong64(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readFloat(const char* key, float& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  constexpr float magic = -0.123456;
  float result = prefs_.getFloat(key, magic);
  if (result == magic) {
    constexpr float magic2 = 1.54e10;
    if (prefs_.getFloat(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readDouble(const char* key, double& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  constexpr double magic = -0.123456;
  double result = prefs_.getDouble(key, magic);
  if (result == magic) {
    constexpr double magic2 = 1.54e10;
    if (prefs_.getDouble(key, magic2) != result) return READ_ERROR;
  }
  val = result;
  return READ_OK;
}

ReadResult PreferencesStore::readString(const char* key, std::string& val) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  size_t size = prefs_.getBytesLength(key);
  if (size == 0) {
    val.clear();
    return READ_OK;
  }
  auto buf = std::unique_ptr<char[]>(new char[size]);
  if (prefs_.getBytes(key, &buf[0], size) == size) {
    val = std::string(&buf[0], size);
    return READ_OK;
  }
  return READ_ERROR;
}

ReadResult PreferencesStore::readBytes(const char* key, void* val,
                                       size_t max_len, size_t* out_len) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  size_t size = prefs_.getBytesLength(key);
  if (out_len != nullptr) *out_len = size;
  if (size == 0) return READ_OK;
  if (size > max_len) return READ_ERROR;
  if (prefs_.getBytes(key, val, size) == size) {
    return READ_OK;
  }
  return READ_ERROR;
}

ReadResult PreferencesStore::readBytesLength(const char* key, size_t* out_len) {
  PreferenceType type = prefs_.getType(key);
  if (type == PT_INVALID) return READ_NOT_FOUND;
  if (type != PT_BLOB) return READ_WRONG_TYPE;
  size_t size = prefs_.getBytesLength(key);
  if (size == 0) return READ_ERROR;
  if (out_len != nullptr) *out_len = size;
  return READ_OK;
}

}  // namespace roo_prefs