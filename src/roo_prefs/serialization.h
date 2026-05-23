#pragma once

#include <inttypes.h>

#include <string>

#include "roo_prefs/store/preferences_store.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace roo_prefs {

using Store = PreferencesStore;

inline ClearResult StoreClear(Store& store, const char* key) {
  return store.clear(key);
}

template <typename T>
inline WriteResult StoreWrite(Store& store, const char* key, const T& val) {
  return store.writeObject(key, val);
}

template <>
inline WriteResult StoreWrite<bool>(Store& store, const char* key,
                                    const bool& val) {
  return store.writeBool(key, val);
}

template <>
inline WriteResult StoreWrite<uint8_t>(Store& store, const char* key,
                                       const uint8_t& val) {
  return store.writeU8(key, val);
}

template <>
inline WriteResult StoreWrite<int8_t>(Store& store, const char* key,
                                      const int8_t& val) {
  return store.writeI8(key, val);
}

template <>
inline WriteResult StoreWrite<uint16_t>(Store& store, const char* key,
                                        const uint16_t& val) {
  return store.writeU16(key, val);
}

template <>
inline WriteResult StoreWrite<int16_t>(Store& store, const char* key,
                                       const int16_t& val) {
  return store.writeI16(key, val);
}

template <>
inline WriteResult StoreWrite<uint32_t>(Store& store, const char* key,
                                        const uint32_t& val) {
  return store.writeU32(key, val);
}

template <>
inline WriteResult StoreWrite<int32_t>(Store& store, const char* key,
                                       const int32_t& val) {
  return store.writeI32(key, val);
}

template <>
inline WriteResult StoreWrite<uint64_t>(Store& store, const char* key,
                                        const uint64_t& val) {
  return store.writeU64(key, val);
}

template <>
inline WriteResult StoreWrite<int64_t>(Store& store, const char* key,
                                       const int64_t& val) {
  return store.writeI64(key, val);
}

template <>
inline WriteResult StoreWrite<float>(Store& store, const char* key,
                                     const float& val) {
  return store.writeFloat(key, val);
}

template <>
inline WriteResult StoreWrite<double>(Store& store, const char* key,
                                      const double& val) {
  return store.writeDouble(key, val);
}

template <>
inline WriteResult StoreWrite<roo::string_view>(Store& store, const char* key,
                                                const roo::string_view& val) {
  return store.writeString(key, val);
}

template <>
inline WriteResult StoreWrite<std::string>(Store& store, const char* key,
                                           const std::string& val) {
  return store.writeString(key, val);
}

template <typename T>
inline ReadResult StoreRead(Store& store, const char* key, T& val) {
  return store.readObject(key, val);
}

template <>
inline ReadResult StoreRead<bool>(Store& store, const char* key, bool& val) {
  return store.readBool(key, val);
}

template <>
inline ReadResult StoreRead<uint8_t>(Store& store, const char* key,
                                     uint8_t& val) {
  return store.readU8(key, val);
}

template <>
inline ReadResult StoreRead<int8_t>(Store& store, const char* key,
                                    int8_t& val) {
  return store.readI8(key, val);
}

template <>
inline ReadResult StoreRead<uint16_t>(Store& store, const char* key,
                                      uint16_t& val) {
  return store.readU16(key, val);
}

template <>
inline ReadResult StoreRead<int16_t>(Store& store, const char* key,
                                     int16_t& val) {
  return store.readI16(key, val);
}

template <>
inline ReadResult StoreRead<uint32_t>(Store& store, const char* key,
                                      uint32_t& val) {
  return store.readU32(key, val);
}

template <>
inline ReadResult StoreRead<int32_t>(Store& store, const char* key,
                                     int32_t& val) {
  return store.readI32(key, val);
}

template <>
inline ReadResult StoreRead<uint64_t>(Store& store, const char* key,
                                      uint64_t& val) {
  return store.readU64(key, val);
}

template <>
inline ReadResult StoreRead<int64_t>(Store& store, const char* key,
                                     int64_t& val) {
  return store.readI64(key, val);
}

template <>
inline ReadResult StoreRead<float>(Store& store, const char* key, float& val) {
  return store.readFloat(key, val);
}

template <>
inline ReadResult StoreRead<double>(Store& store, const char* key,
                                    double& val) {
  return store.readDouble(key, val);
}

template <>
inline ReadResult StoreRead<std::string>(Store& store, const char* key,
                                         std::string& val) {
  return store.readString(key, val);
}

#ifdef ARDUINO
template <>
inline WriteResult StoreWrite<String>(Store& store, const char* key,
                                      const String& val) {
  return store.writeString(key, val.c_str());
}

template <>
inline ReadResult StoreRead<String>(Store& store, const char* key,
                                    String& val) {
  std::string temp;
  ReadResult result = store.readString(key, temp);
  if (result == ReadResult::kOk) {
    val = String(temp.c_str());
  }
  return result;
}

#endif

}  // namespace roo_prefs