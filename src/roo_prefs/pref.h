#pragma once

#include <inttypes.h>

#include <string>

#include "roo_prefs/collection.h"
#include "roo_prefs/serialization.h"
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

namespace internal {

static roo::string_view ToStringView(roo::string_view value) { return value; }

static roo::string_view ToStringView(const std::string& value) {
  return roo::string_view(value.c_str(), value.size());
}

static roo::string_view ToStringView(const char* value) {
  return roo::string_view((value == nullptr) ? "" : value);
}

template <size_t N>
static roo::string_view ToStringView(const char (&value)[N]) {
  size_t size = N;
  if (size > 0 && value[size - 1] == '\0') {
    --size;
  }
  return roo::string_view(value, size);
}

#ifdef ARDUINO
static roo::string_view ToStringView(const ::String& value) {
  return roo::string_view(value.c_str(), value.length());
}
#endif

template <typename V>
class ValueHolder {
 public:
  ValueHolder(const V& other = V()) : value_(other) {}
  ValueHolder(V&& other) : value_(std::move(other)) {}

  const V& get() const { return value_; }
  V& get() { return value_; }

  void set(const V& value) { value_ = value; }

  bool equals(const V& other) const { return value_ == other; }

  static const V& Assignable(const V& value) { return value; }

 private:
  V value_;
};

template <>
class ValueHolder<std::string> {
 public:
  ValueHolder(const std::string& other = std::string()) : value_(other) {}
  ValueHolder(std::string&& other) : value_(std::move(other)) {}

  template <typename V>
  ValueHolder(const V& other) : value_(ToStringView(other)) {}

  const std::string& get() const { return value_; }

  std::string& get() { return value_; }

  void set(const std::string& value) { value_ = value; }
  void set(std::string&& value) { value_ = std::move(value); }

  bool equals(const std::string& other) const { return value_ == other; }

  static const std::string& Assignable(const std::string& value) {
    return value;
  }

  void set(roo::string_view value) {
    if (value.empty()) {
      value_.clear();
      return;
    }
    value_.assign(value.data(), value.size());
  }

  bool equals(roo::string_view other) const {
    if (value_.size() != other.size()) {
      return false;
    }
    if (other.empty()) {
      return true;
    }
    return std::char_traits<char>::compare(value_.data(), other.data(),
                                           other.size()) == 0;
  }

  static roo::string_view Assignable(roo::string_view value) { return value; }

  template <typename V>
  void set(const V& value) {
    set(ToStringView(value));
  }

  template <typename V>
  bool equals(const V& other) const {
    return equals(ToStringView(other));
  }

  template <typename V>
  static roo::string_view Assignable(const V& value) {
    return ToStringView(value);
  }

 private:
  std::string value_;
};

#ifdef ARDUINO
template <>
class ValueHolder<::String> {
 public:
  ValueHolder(const ::String& other = ::String()) : value_(other) {}

  template <typename V>
  ValueHolder(const V& other) : value_(ToStringView(other)) {}

  const ::String& get() const { return value_; }
  ::String& get() { return value_; }

  void set(const ::String& value) { value_ = value; }

  void set(roo::string_view value) {
    if (value.empty()) {
      value_.clear();
      return;
    }
    value_ = ::String(value.data(), value.size());
  }

  bool equals(roo::string_view other) const {
    if (value_.length() != other.size()) {
      return false;
    }
    if (other.empty()) {
      return true;
    }
    return std::char_traits<char>::compare(value_.c_str(), other.data(),
                                           other.size()) == 0;
  }

  template <typename V>
  void set(const V& value) {
    set(ToStringView(value));
  }

  template <typename V>
  bool equals(const V& other) const {
    return equals(ToStringView(other));
  }

  template <typename V>
  static roo::string_view Assignable(const V& value) {
    return ToStringView(value);
  }

 private:
  ::String value_;
};
#endif

}  // namespace internal

template <typename T>
class Pref {
 public:
  Pref(Collection& collection, const char* key, T default_value = T());

  bool isSet() const;

  const T& get() const;

  template <typename V = T>
  bool set(const V& value);

  bool clear();

 private:
  enum class PrefState { kUnknown, kUnset, kSet, kError };

  void sync() const;

  Collection& collection_;
  const char* key_;
  T default_value_;
  mutable PrefState state_;
  mutable internal::ValueHolder<T> value_;
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

using StdString = Pref<std::string>;

#ifdef ARDUINO
using ArduinoString = Pref<::String>;
#endif

using String = StdString;

/// Implementation details follow.

template <typename T>
Pref<T>::Pref(Collection& collection, const char* key, T default_value)
    : collection_(collection),
      key_(key),
      default_value_(std::move(default_value)),
      state_(PrefState::kUnknown),
      value_(default_value) {}

template <typename T>
bool Pref<T>::isSet() const {
  sync();
  return (state_ == PrefState::kSet);
}

template <typename T>
const T& Pref<T>::get() const {
  sync();
  return value_.get();
}

template <typename T>
template <typename V>
bool Pref<T>::set(const V& value) {
  sync();
  if (state_ == PrefState::kSet && value_.equals(value)) {
    return true;
  }
  Transaction t(collection_);
  if (!t.active()) {
    state_ = PrefState::kError;
    return false;
  }
  switch (StoreWrite(t.store(), key_,
                     internal::ValueHolder<T>::Assignable(value))) {
    case WriteResult::kOk: {
      value_.set(value);
      state_ = PrefState::kSet;
      return true;
    }
    default: {
      state_ = PrefState::kError;
      return false;
    }
  }
}

template <typename T>
bool Pref<T>::clear() {
  sync();
  if (state_ == PrefState::kUnset) {
    return true;
  }
  Transaction t(collection_);
  if (!t.active()) {
    state_ = PrefState::kError;
    return false;
  }
  switch (StoreClear(t.store(), key_)) {
    case ClearResult::kOk: {
      state_ = PrefState::kUnset;
      value_.set(default_value_);
      return true;
    }
    default: {
      state_ = PrefState::kError;
      return false;
    }
  }
}

template <typename T>
void Pref<T>::sync() const {
  if (state_ == PrefState::kUnknown || state_ == PrefState::kError) {
    Transaction t(collection_, true);
    if (!t.active()) {
      state_ = PrefState::kUnset;
      value_.set(default_value_);
      return;
    }
    switch (StoreRead(t.store(), key_, value_.get())) {
      case ReadResult::kOk: {
        state_ = PrefState::kSet;
        return;
      }
      case ReadResult::kNotFound: {
        state_ = PrefState::kUnset;
        value_.set(default_value_);
        return;
      }
      default: {
        state_ = PrefState::kError;
        return;
      }
    }
  }
}

}  // namespace roo_prefs