#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "roo_backport/string_view.h"
#include "roo_prefs/status.h"

namespace roo_prefs {

class Collection;

using KeyVisitor = bool (*)(void* context, roo::string_view key);

/// Backend-independent low-level preferences store.
///
/// A store instance may be attached to one Collection at a time. Collection
/// controls the begin/end lifecycle through Transaction.
class Store {
 public:
  enum class BeginResult { kOk, kNotFound, kError };

  virtual ~Store() = default;

  virtual bool isKey(const char* key) = 0;
  virtual ClearResult clear(const char* key) = 0;

  template <typename T>
  WriteResult writeObject(const char* key, const T& val) {
    return writeObjectInternal(key, &val, sizeof(val));
  }

  virtual WriteResult writeBool(const char* key, bool val) = 0;
  virtual WriteResult writeU8(const char* key, uint8_t val) = 0;
  virtual WriteResult writeI8(const char* key, int8_t val) = 0;
  virtual WriteResult writeU16(const char* key, uint16_t val) = 0;
  virtual WriteResult writeI16(const char* key, int16_t val) = 0;
  virtual WriteResult writeU32(const char* key, uint32_t val) = 0;
  virtual WriteResult writeI32(const char* key, int32_t val) = 0;
  virtual WriteResult writeU64(const char* key, uint64_t val) = 0;
  virtual WriteResult writeI64(const char* key, int64_t val) = 0;
  virtual WriteResult writeFloat(const char* key, float val) = 0;
  virtual WriteResult writeDouble(const char* key, double val) = 0;
  virtual WriteResult writeString(const char* key, roo::string_view val) = 0;
  virtual WriteResult writeBytes(const char* key, const void* val,
                                 size_t len) = 0;

  template <typename T>
  ReadResult readObject(const char* key, T& val) {
    return readObjectInternal(key, &val, sizeof(val));
  }

  virtual ReadResult readBool(const char* key, bool& val) = 0;
  virtual ReadResult readU8(const char* key, uint8_t& val) = 0;
  virtual ReadResult readI8(const char* key, int8_t& val) = 0;
  virtual ReadResult readU16(const char* key, uint16_t& val) = 0;
  virtual ReadResult readI16(const char* key, int16_t& val) = 0;
  virtual ReadResult readU32(const char* key, uint32_t& val) = 0;
  virtual ReadResult readI32(const char* key, int32_t& val) = 0;
  virtual ReadResult readU64(const char* key, uint64_t& val) = 0;
  virtual ReadResult readI64(const char* key, int64_t& val) = 0;
  virtual ReadResult readFloat(const char* key, float& val) = 0;
  virtual ReadResult readDouble(const char* key, double& val) = 0;
  virtual ReadResult readString(const char* key, std::string& val) = 0;
  virtual ReadResult readBytes(const char* key, void* val, size_t max_len,
                               size_t* out_len) = 0;
  virtual ReadResult readBytesLength(const char* key, size_t* out_len) = 0;

 private:
  friend class Collection;

  virtual BeginResult begin(const char* collection_name, bool read_only) = 0;
  virtual void end() = 0;
  virtual EnumerateResult enumerateKeys(const char* collection_name,
                                        KeyVisitor visitor,
                                        void* context) const = 0;
  virtual WriteResult writeObjectInternal(const char* key, const void* val,
                                          size_t size) = 0;
  virtual ReadResult readObjectInternal(const char* key, void* val,
                                        size_t size) = 0;
};

}  // namespace roo_prefs
