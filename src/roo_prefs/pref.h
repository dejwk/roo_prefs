#pragma once

#include <inttypes.h>

#include <string>

#include "roo_prefs/collection.h"
#include "roo_prefs/status.h"
#include "roo_prefs/transaction.h"

namespace roo_prefs {

/// Persistent preference of a specific type.
/// The preference will store its value in the preferences collection provided
/// in the constructor, under the specified key (which needs to remain
/// constant). The value is read lazily (on first access.) You can provide
/// default value to be used if the collection does not contain the specified
/// key. The value is cached in memory - subsequent reads will return the cached
/// value. Writes always write to the persistent storage (and also, update the
/// cache.) Read and write are performed within implicitly created
/// transactions. If you have multiple properties that you want to read/write at
/// the same time, you can read them under an explicit transaction:
///
/// roo_prefs::Collection col("foo");
/// roo_prefs::Int16 pref1(col, "pref1");
/// roo_prefs::String pref2(col, "pref2");
/// int16_t a;
/// std::string b;
/// {
///   Transaction t(col);
///   a = pref1.get();
///   b = pref2.get();
/// }
///
/// For simple types, use aliases defined later in the file (Uint8, String,
/// etc.)
///
/// You can persist arbitrary types (e.g. struct types). Types other than simple
/// types and std::string are persisted as BLOBs, containing the binary
/// representation of the object.
///
/// NOTE:
/// * Preferences library is generally intended for storing small values.
/// * If the definition (and thus the internal representation) of your complex
///   type changes, any persisted values of that type will become unreadable or
///   corrupted.
/// For these reasons, it is generally not a good practice to persist large
/// objects using this template.
///
template <typename T>
class Pref {
 public:
  Pref(Collection& collection, const char* key, T default_value = T());

  bool isSet() const;

  const T& get() const;

  bool set(const T& value);

  bool clear();

 private:
  enum PrefState { UNKNOWN, UNSET, SET, ERROR };

  void sync() const;

  Collection& collection_;
  const char* key_;
  T default_value_;
  mutable PrefState state_;
  mutable T value_;
};

using Bool = Pref<bool>;
using Uint8 = Pref<uint8_t>;
using Int8 = Pref<int8_t>;
using Uint16 = Pref<uint16_t>;
using Int16 = Pref<int16_t>;
using Uint32 = Pref<uint32_t>;
using Int32 = Pref<int32_t>;
using Uint64 = Pref<uint64_t>;
using Int64 = Pref<int64_t>;
using Float = Pref<float>;
using Double = Pref<double>;

using String = Pref<std::string>;

/// Implementation details follow.

namespace internal {

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

}  // namespace internal

template <typename T>
Pref<T>::Pref(Collection& collection, const char* key, T default_value)
    : collection_(collection),
      key_(key),
      default_value_(std::move(default_value)),
      state_(UNKNOWN),
      value_(default_value) {}

template <typename T>
bool Pref<T>::isSet() const {
  sync();
  return (state_ == SET);
}

template <typename T>
const T& Pref<T>::get() const {
  sync();
  return value_;
}

template <typename T>
bool Pref<T>::set(const T& value) {
  sync();
  if (state_ == SET && value_ == value) {
    return true;
  }
  Transaction t(collection_);
  if (!t.active()) {
    state_ = ERROR;
    return false;
  }
  switch (internal::StoreWrite(t.store(), key_, value)) {
    case WRITE_OK: {
      value_ = value;
      state_ = SET;
      return true;
    }
    default: {
      state_ = ERROR;
      return false;
    }
  }
}

template <typename T>
bool Pref<T>::clear() {
  sync();
  if (state_ == UNSET) {
    return true;
  }
  Transaction t(collection_);
  if (!t.active()) {
    state_ = ERROR;
    return false;
  }
  switch (internal::StoreClear(t.store(), key_)) {
    case CLEAR_OK: {
      state_ = UNSET;
      value_ = default_value_;
      return true;
    }
    default: {
      state_ = ERROR;
      return false;
    }
  }
}

template <typename T>
void Pref<T>::sync() const {
  if (state_ == UNKNOWN || state_ == ERROR) {
    Transaction t(collection_, true);
    if (!t.active()) {
      state_ = UNSET;
      value_ = default_value_;
      return;
    }
    switch (internal::StoreRead(t.store(), key_, value_)) {
      case READ_OK: {
        state_ = SET;
        return;
      }
      case READ_NOT_FOUND: {
        state_ = UNSET;
        value_ = default_value_;
        return;
      }
      default: {
        state_ = ERROR;
        return;
      }
    }
  }
}

}  // namespace roo_prefs