#pragma once

#include <inttypes.h>

#include <functional>
#include <string>

#include "Preferences.h"
#include "roo_logging.h"

namespace roo_prefs {

// Basic usage:
//
// roo_prefs::Collection c("col-name");  // Statically declared.
//
// {
//   roo_prefs::Transaction t(c);
//   t.store().putInt("pref-name", val);
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
//   t.putInt(key, val);
// }
//
// // ...
// {
//   roo_prefs::Transaction t(c);
//   foo("a", 1);
//   foo("b", 2);
// }

class Transaction;

using Store = Preferences;

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

enum PrefState { UNKNOWN, UNSET, SET, ERROR };

enum ReadResult { READ_OK, READ_OK_UNSET, READ_ERROR };
enum WriteResult { WRITE_OK, WRITE_ERROR };
enum ClearResult { CLEAR_OK, CLEAR_ERROR };

inline ClearResult StoreClear(Store& store, const char* key) {
  return store.remove(key) ? CLEAR_OK : CLEAR_ERROR;
}

template <typename T>
inline WriteResult StoreWrite(Store& store, const char* key, const T& val) {
  return (store.putBytes(key, &val, sizeof(val)) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<uint8_t>(Store& store, const char* key,
                                       const uint8_t& val) {
  return (store.putUChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<int8_t>(Store& store, const char* key,
                                      const int8_t& val) {
  return (store.putChar(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<uint16_t>(Store& store, const char* key,
                                        const uint16_t& val) {
  return (store.putUShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<int16_t>(Store& store, const char* key,
                                       const int16_t& val) {
  return (store.putShort(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<uint32_t>(Store& store, const char* key,
                                        const uint32_t& val) {
  return (store.putULong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<int32_t>(Store& store, const char* key,
                                       const int32_t& val) {
  return (store.putLong(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<uint64_t>(Store& store, const char* key,
                                        const uint64_t& val) {
  return (store.putULong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<int64_t>(Store& store, const char* key,
                                       const int64_t& val) {
  return (store.putLong64(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<float>(Store& store, const char* key,
                                     const float& val) {
  return (store.putFloat(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<double>(Store& store, const char* key,
                                      const double& val) {
  return (store.putDouble(key, val) > 0) ? WRITE_OK : WRITE_ERROR;
}

template <>
inline WriteResult StoreWrite<std::string>(Store& store, const char* key,
                                           const std::string& val) {
  return (store.putBytes(key, val.data(), val.size()) > 0) ? WRITE_OK
                                                           : WRITE_ERROR;
}

template <typename T>
inline ReadResult StoreRead(Store& store, const char* key, T& val) {
  if (store.getType(key) != PT_BLOB ||
      store.getBytesLength(key) != sizeof(val)) {
    return READ_OK_UNSET;
  } else {
    if (store.getBytes(key, &val, sizeof(val)) == sizeof(val)) {
      return READ_OK;
    } else {
      LOG(ERROR) << "Failed to read prefs key " << key;
      return READ_ERROR;
    }
  }
}

template <>
inline ReadResult StoreRead<uint8_t>(Store& store, const char* key,
                                     uint8_t& val) {
  if (store.getType(key) != PT_U8) return READ_OK_UNSET;
  constexpr uint8_t magic = 0xDF;
  val = store.getUChar(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<int8_t>(Store& store, const char* key,
                                    int8_t& val) {
  if (store.getType(key) != PT_I8) return READ_OK_UNSET;
  constexpr int8_t magic = 0xDF;
  val = store.getChar(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<uint16_t>(Store& store, const char* key,
                                      uint16_t& val) {
  if (store.getType(key) != PT_U16) return READ_OK_UNSET;
  constexpr uint16_t magic = 0xDFB1;
  val = store.getUShort(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<int16_t>(Store& store, const char* key,
                                     int16_t& val) {
  if (store.getType(key) != PT_I16) return READ_OK_UNSET;
  constexpr int16_t magic = 0xDFB1;
  val = store.getShort(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<uint32_t>(Store& store, const char* key,
                                      uint32_t& val) {
  if (store.getType(key) != PT_U32) return READ_OK_UNSET;
  constexpr uint32_t magic = 0xDFB1BEEF;
  val = store.getULong(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<int32_t>(Store& store, const char* key,
                                     int32_t& val) {
  if (store.getType(key) != PT_I32) return READ_OK_UNSET;
  constexpr int32_t magic = 0xDFB1BEEF;
  val = store.getLong(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<uint64_t>(Store& store, const char* key,
                                      uint64_t& val) {
  if (store.getType(key) != PT_U64) return READ_OK_UNSET;
  constexpr uint64_t magic = 0x3E3E1254DFB1BEEF;
  val = store.getULong64(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<int64_t>(Store& store, const char* key,
                                     int64_t& val) {
  if (store.getType(key) != PT_I64) return READ_OK_UNSET;
  constexpr int64_t magic = 0x3E3E1254DFB1BEEF;
  val = store.getLong64(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<float>(Store& store, const char* key, float& val) {
  constexpr float magic = -0.123456;
  val = store.getFloat(key, magic);
  if (val == magic && store.getType(key) != PT_BLOB) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<double>(Store& store, const char* key,
                                    double& val) {
  if (store.getType(key) != PT_BLOB) return READ_OK_UNSET;
  constexpr double magic = -0.123456;
  val = store.getDouble(key, magic);
  if (val == magic) {
    return READ_OK_UNSET;
  } else {
    return READ_OK;
  }
}

template <>
inline ReadResult StoreRead<std::string>(Store& store, const char* key,
                                         std::string& val) {
  if (store.getType(key) != PT_BLOB) {
    return READ_OK_UNSET;
  }
  size_t size = store.getBytesLength(key);
  val.resize(size);
  if (size == 0) return READ_OK;
  if (store.getBytes(key, &*val.begin(), val.size()) == val.size()) {
    return READ_OK;
  }
  LOG(ERROR) << "Failed to read prefs key " << key;
  return READ_ERROR;
}

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
    switch (StoreWrite(t.store(), key_, value)) {
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
    switch (StoreClear(t.store(), key_)) {
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
      switch (StoreRead(t.store(), key_, value_)) {
        case READ_OK: {
          state_ = SET;
          return;
        }
        case READ_OK_UNSET: {
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