#pragma once

#include <inttypes.h>

#include <functional>
#include <memory>
#include <string>

#include "Preferences.h"
#include "roo_backport.h"
#include "roo_logging.h"

namespace roo_prefs {

// Basic usage:
//
// roo_prefs::Collection c("col-name");  // Statically declared.
//
// {
//   roo_prefs::Transaction t(c);
//   t.store().writeI32("pref-name", val);
// }
//
// In the snippet above, Preferences::begin()/end() will be called automatically
// by the transaction constructor/destructor.
//
// If you would like to use automated in-memory caching, use typed 'Pref'
// classes declared later in this file instead of creating transactions
// directly.
//
// Note: transactions have reference-counting (and are reentrant). In other
// words, this works, and only calls the begin()/end() pair once:
//
// void foo(const string& key, int val) {
//   roo_prefs::Transaction t(c);
//   t.writeI32(key, val);
// }
//
// // ...
// {
//   roo_prefs::Transaction t(c);
//   foo("a", 1);
//   foo("b", 2);
// }

class Transaction;

enum PrefState { UNKNOWN, UNSET, SET, ERROR };

enum ReadResult { READ_OK, READ_NOT_FOUND, READ_WRONG_TYPE, READ_ERROR };
enum WriteResult { WRITE_OK, WRITE_ERROR };
enum ClearResult { CLEAR_OK, CLEAR_ERROR };

class PreferencesStore {
 public:
  bool begin(const char* collection_name, bool read_only) {
    return prefs_.begin(collection_name, read_only);
  }

  void end() { prefs_.end(); }

  bool isKey(const char* key) { return prefs_.isKey(key); }

  ClearResult clear(const char* key) {
    return prefs_.remove(key) ? CLEAR_OK : CLEAR_ERROR;
  }

  template <typename T>
  WriteResult writeObject(const char* key, const T& val) {
    return (prefs_.putBytes(key, &val, sizeof(val)) > 0) ? WRITE_OK
                                                         : WRITE_ERROR;
  }

  WriteResult writeBool(const char* key, bool val) {
    return (prefs_.putBool(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeU8(const char* key, uint8_t val) {
    return (prefs_.putUChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeI8(const char* key, int8_t val) {
    return (prefs_.putChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeU16(const char* key, uint16_t val) {
    return (prefs_.putUShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  inline WriteResult writeI16(const char* key, int16_t val) {
    return (prefs_.putShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeU32(const char* key, uint32_t val) {
    return (prefs_.putULong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeI32(const char* key, int32_t val) {
    return (prefs_.putLong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeU64(const char* key, uint64_t val) {
    return (prefs_.putULong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeI64(const char* key, int64_t val) {
    return (prefs_.putLong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeFloat(const char* key, float val) {
    return (prefs_.putFloat(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  WriteResult writeDouble(const char* key, double val) {
    return (prefs_.putDouble(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
  }

  inline WriteResult writeString(const char* key, roo::string_view val) {
    return (prefs_.putBytes(key, val.data(), val.size()) > 0) ? WRITE_OK
                                                              : WRITE_ERROR;
  }

  template <typename T>
  inline ReadResult readObject(const char* key, T& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_BLOB) return READ_WRONG_TYPE;
    if (prefs_.getBytesLength(key) != sizeof(val)) {
      return READ_WRONG_TYPE;
    }
    if (prefs_.getBytes(key, &val, sizeof(val)) != sizeof(val)) {
      return READ_ERROR;
    }
    return READ_OK;
  }

  ReadResult readBool(const char* key, bool& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_U8) return READ_WRONG_TYPE;
    bool v1 = prefs_.getBool(key, false);
    bool v2 = prefs_.getBool(key, true);
    if (v1 != v2) {
      return READ_ERROR;
    } else {
      val = v1;
      return READ_OK;
    }
  }

  ReadResult readU8(const char* key, uint8_t& val) {
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

  ReadResult readI8(const char* key, int8_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_I8) return READ_WRONG_TYPE;
    constexpr int8_t magic = 0xDF;
    int8_t result = prefs_.getChar(key, magic);
    if (result == magic) {
      constexpr int8_t magic2 = 0x12;
      if (prefs_.getChar(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readU16(const char* key, uint16_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_U16) return READ_WRONG_TYPE;
    constexpr uint16_t magic = 0xDFB1;
    uint16_t result = prefs_.getUShort(key, magic);
    if (result == magic) {
      constexpr uint16_t magic2 = 0x1223;
      if (prefs_.getUShort(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readI16(const char* key, int16_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_I16) return READ_WRONG_TYPE;
    constexpr int16_t magic = 0xDFB1;
    int16_t result = prefs_.getShort(key, magic);
    if (result == magic) {
      constexpr int16_t magic2 = 0x1223;
      if (prefs_.getShort(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readU32(const char* key, uint32_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_U32) return READ_WRONG_TYPE;
    constexpr uint32_t magic = 0xDFB1BEEF;
    uint32_t result = prefs_.getULong(key, magic);
    if (result == magic) {
      constexpr uint32_t magic2 = 0x12345678;
      if (prefs_.getULong(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readI32(const char* key, int32_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_I32) return READ_WRONG_TYPE;
    constexpr int32_t magic = 0xDFB1BEEF;
    int32_t result = prefs_.getLong(key, magic);
    if (result == magic) {
      constexpr int32_t magic2 = 0x12345678;
      if (prefs_.getLong(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readU64(const char* key, uint64_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_U64) return READ_WRONG_TYPE;
    constexpr uint64_t magic = 0x3E3E1254DFB1BEEFLL;
    uint64_t result = prefs_.getULong64(key, magic);
    if (result == magic) {
      constexpr uint64_t magic2 = 0x1234567812345678LL;
      if (prefs_.getULong64(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readI64(const char* key, int64_t& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_I64) return READ_WRONG_TYPE;
    constexpr int64_t magic = 0x3E3E1254DFB1BEEF;
    int64_t result = prefs_.getLong64(key, magic);
    if (result == magic) {
      constexpr int64_t magic2 = 0x1234567812345678LL;
      if (prefs_.getLong64(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readFloat(const char* key, float& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_BLOB) return READ_WRONG_TYPE;
    constexpr float magic = -0.123456;
    float result = prefs_.getFloat(key, magic);
    if (result == magic) {
      constexpr float magic2 = 1.54e10;
      if (prefs_.getFloat(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readDouble(const char* key, double& val) {
    PreferenceType type = prefs_.getType(key);
    if (type == PT_INVALID) return READ_NOT_FOUND;
    if (type != PT_BLOB) return READ_WRONG_TYPE;
    constexpr double magic = -0.123456;
    double result = prefs_.getDouble(key, magic);
    if (result == magic) {
      constexpr double magic2 = 1.54e10;
      if (prefs_.getDouble(key, magic2) != result) return READ_ERROR;
    } else {
      val = result;
      return READ_OK;
    }
  }

  ReadResult readString(const char* key, std::string& val) {
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

  ReadResult readBytes(const char* key, void* val, size_t max_len,
                       size_t* out_len) {
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

 private:
  Preferences prefs_;
};

using Store = PreferencesStore;

// Collection corresponds to a preferences namespace. Use it to group related
// preferences.
class Collection {
 public:
  Collection(const char* name)
      : store_(), name_(name), refcount_(0), read_only_(true) {}

 private:
  friend class Transaction;

  bool inc(bool read_only) {
    if (refcount_ == 0) {
      if (!store_.begin(name_, read_only)) {
        if (read_only) {
          LOG(WARNING) << "Failed to initialize preferences " << name_
                       << " for reading";
        } else {
          LOG(ERROR) << "Failed to initialize preferences " << name_
                     << " for writing";
        }
        return false;
      }
      read_only_ = read_only;
      ++refcount_;
      return true;
    }
    if (read_only_ && !read_only) return false;
    ++refcount_;
    return true;
  }

  void dec() {
    if (--refcount_ == 0) {
      store_.end();
    }
  }

  Store store_;
  const char* name_;
  int refcount_;
  bool read_only_;
};

// Ref-counted RAII for managing access to Preference namespaces. Allows
// orchestrating access to the Store.
class Transaction {
 public:
  Transaction(Collection& collection, bool read_only = false)
      : collection_(collection) {
    active_ = collection_.inc(read_only);
  }

  ~Transaction() {
    if (active_) collection_.dec();
  }

  bool active() const { return active_; }

  Store& store() { return collection_.store_; }

 private:
  Collection& collection_;
  bool active_;
};

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

// Persistent preference of a specific type.
// The preference will store its value in the preferences collection provided in
// the constructor, under the specified key (which needs to remain constant).
// The value is read lazily (on first access.) You can provide default value to
// be used if the collection does not contain the specified key. The value is
// cached in memory - subsequent reads will return the cached value. Writes
// always write to the persistent storage (and also, update the cache.) Read and
// write are performed within implicitly created transactions. If you have
// multiple properties that you want to read/write at the same time, you can
// read them under an explicit transaction:
//
// roo_prefs::Collection col("foo");
// roo_prefs::Int16 pref1(col, "pref1");
// roo_prefs::String pref2(col, "pref2");
// int16_t a;
// std::string b;
// {
//   Transaction t(collection);
//   a = pref1.get();
//   b = pref2.get();
// }
//
// For simple types, use  aliases defined later in the file (Uint8, String,
// etc.)
//
// You can persist arbitrary types (e.g. struct types). Types other than simple
// types and std::string are persisted as BLOBs, containing the binary
// representation of the object.
//
// NOTE:
// * Preferences library is generally intended for storing small values.
// * If the definition (and thus the internal representation) of your complex
//   type changes, any persisted values of that type will become unreadable or
//   corrupted.
// For these reasons, it is generally not a good practice to persist large
// objects using this template.
//
template <typename T>
class Pref {
 public:
  Pref(Collection& collection, const char* key, T default_value = T())
      : collection_(collection),
        key_(key),
        default_value_(std::move(default_value)),
        state_(UNKNOWN),
        value_(default_value) {}

  bool isSet() const {
    sync();
    return (state_ == SET);
  }

  const T& get() const {
    sync();
    return value_;
  }

  bool set(const T& value) {
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

  bool clear() {
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
        return true;
      }
      default: {
        state_ = ERROR;
        return false;
      }
    }
  }

 private:
  void sync() const {
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

}  // namespace roo_prefs