#pragma once

#include <string>

#include "roo_backport.h"
#include "roo_backport/string_view.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace roo_prefs {

namespace internal {

inline roo::string_view ToStringView(roo::string_view value) { return value; }

inline roo::string_view ToStringView(const std::string& value) {
  return roo::string_view(value.c_str(), value.size());
}

inline roo::string_view ToStringView(const char* value) {
  return roo::string_view((value == nullptr) ? "" : value);
}

template <size_t N>
inline roo::string_view ToStringView(const char (&value)[N]) {
  size_t size = N;
  if (size > 0 && value[size - 1] == '\0') {
    --size;
  }
  return roo::string_view(value, size);
}

#ifdef ARDUINO
inline roo::string_view ToStringView(const ::String& value) {
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

}  // namespace roo_prefs