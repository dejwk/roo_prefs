#include "roo_prefs/store/nvs_store.h"

#if ROO_PREFS_USE_ESP32_NVS

#include <esp_idf_version.h>

#include <memory>

namespace roo_prefs {
namespace {

ReadResult ToReadResult(esp_err_t result) {
  switch (result) {
    case ESP_OK:
      return ReadResult::kOk;
    case ESP_ERR_NVS_NOT_FOUND:
      return ReadResult::kNotFound;
    case ESP_ERR_NVS_TYPE_MISMATCH:
      return ReadResult::kWrongType;
    default:
      return ReadResult::kError;
  }
}

WriteResult CommitWrite(nvs_handle_t handle, esp_err_t result) {
  if (result != ESP_OK) return WriteResult::kError;
  return nvs_commit(handle) == ESP_OK ? WriteResult::kOk : WriteResult::kError;
}

template <typename T>
ReadResult ReadScalar(esp_err_t (*getter)(nvs_handle_t, const char*, T*),
                      nvs_handle_t handle, const char* key, T& val) {
  T result;
  ReadResult status = ToReadResult(getter(handle, key, &result));
  if (status == ReadResult::kOk) val = result;
  return status;
}

bool HasAnyType(nvs_handle_t handle, const char* key) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
  return nvs_find_key(handle, key, nullptr) == ESP_OK;
#else
  // nvs_find_key() is unavailable in ESP-IDF 4.x. Probe every NVS type to
  // retain compatibility with Arduino-ESP32 2.x and other older SDKs.
  int8_t i8;
  uint8_t u8;
  int16_t i16;
  uint16_t u16;
  int32_t i32;
  uint32_t u32;
  int64_t i64;
  uint64_t u64;
  size_t len = 0;
  return nvs_get_i8(handle, key, &i8) == ESP_OK ||
         nvs_get_u8(handle, key, &u8) == ESP_OK ||
         nvs_get_i16(handle, key, &i16) == ESP_OK ||
         nvs_get_u16(handle, key, &u16) == ESP_OK ||
         nvs_get_i32(handle, key, &i32) == ESP_OK ||
         nvs_get_u32(handle, key, &u32) == ESP_OK ||
         nvs_get_i64(handle, key, &i64) == ESP_OK ||
         nvs_get_u64(handle, key, &u64) == ESP_OK ||
         nvs_get_str(handle, key, nullptr, &len) == ESP_OK ||
         nvs_get_blob(handle, key, nullptr, &len) == ESP_OK;
#endif
}

}  // namespace

NvsStore::~NvsStore() { end(); }

Store::BeginResult NvsStore::begin(const char* collection_name,
                                   bool read_only) {
  if (open_) return BeginResult::kError;
  esp_err_t result = nvs_open(
      collection_name, read_only ? NVS_READONLY : NVS_READWRITE, &handle_);
  if (result == ESP_ERR_NVS_NOT_FOUND && read_only) {
    return BeginResult::kNotFound;
  }
  if (result != ESP_OK) return BeginResult::kError;
  open_ = true;
  return BeginResult::kOk;
}

void NvsStore::end() {
  if (!open_) return;
  nvs_close(handle_);
  handle_ = 0;
  open_ = false;
}

EnumerateResult NvsStore::enumerateKeys(const char* collection_name,
                                        KeyVisitor visitor,
                                        void* context) const {
  if (collection_name == nullptr || visitor == nullptr) {
    return EnumerateResult::kError;
  }

  // Verify the namespace separately so ESP-IDF 4.x can distinguish an absent
  // namespace (an empty result) from a storage initialization failure.
  nvs_handle_t check_handle;
  esp_err_t result = nvs_open(collection_name, NVS_READONLY, &check_handle);
  if (result == ESP_ERR_NVS_NOT_FOUND) return EnumerateResult::kOk;
  if (result != ESP_OK) return EnumerateResult::kError;
  nvs_close(check_handle);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  nvs_iterator_t iterator = nullptr;
  result = nvs_entry_find("nvs", collection_name, NVS_TYPE_ANY, &iterator);
  if (result == ESP_ERR_NVS_NOT_FOUND) return EnumerateResult::kOk;
  if (result != ESP_OK) return EnumerateResult::kError;

  while (result == ESP_OK) {
    nvs_entry_info_t info;
    if (nvs_entry_info(iterator, &info) != ESP_OK) {
      nvs_release_iterator(iterator);
      return EnumerateResult::kError;
    }
    if (!visitor(context, roo::string_view(info.key))) {
      nvs_release_iterator(iterator);
      return EnumerateResult::kStopped;
    }
    result = nvs_entry_next(&iterator);
  }
  nvs_release_iterator(iterator);
  return result == ESP_ERR_NVS_NOT_FOUND ? EnumerateResult::kOk
                                         : EnumerateResult::kError;
#else
  nvs_iterator_t iterator =
      nvs_entry_find("nvs", collection_name, NVS_TYPE_ANY);
  while (iterator != nullptr) {
    nvs_entry_info_t info;
    nvs_entry_info(iterator, &info);
    if (!visitor(context, roo::string_view(info.key))) {
      nvs_release_iterator(iterator);
      return EnumerateResult::kStopped;
    }
    iterator = nvs_entry_next(iterator);
  }
  return EnumerateResult::kOk;
#endif
}

bool NvsStore::isKey(const char* key) {
  return open_ && key != nullptr && HasAnyType(handle_, key);
}

ClearResult NvsStore::clear(const char* key) {
  if (!open_ || key == nullptr || nvs_erase_key(handle_, key) != ESP_OK) {
    return ClearResult::kError;
  }
  return nvs_commit(handle_) == ESP_OK ? ClearResult::kOk : ClearResult::kError;
}

WriteResult NvsStore::writeBytes(const char* key, const void* val, size_t len) {
  if (!open_ || key == nullptr || val == nullptr || len == 0) {
    return WriteResult::kError;
  }
  return CommitWrite(handle_, nvs_set_blob(handle_, key, val, len));
}

WriteResult NvsStore::writeObjectInternal(const char* key, const void* val,
                                          size_t size) {
  return writeBytes(key, val, size);
}

WriteResult NvsStore::writeBool(const char* key, bool val) {
  return writeU8(key, val ? 1 : 0);
}

WriteResult NvsStore::writeU8(const char* key, uint8_t val) {
  return CommitWrite(handle_, nvs_set_u8(handle_, key, val));
}

WriteResult NvsStore::writeI8(const char* key, int8_t val) {
  return CommitWrite(handle_, nvs_set_i8(handle_, key, val));
}

WriteResult NvsStore::writeU16(const char* key, uint16_t val) {
  return CommitWrite(handle_, nvs_set_u16(handle_, key, val));
}

WriteResult NvsStore::writeI16(const char* key, int16_t val) {
  return CommitWrite(handle_, nvs_set_i16(handle_, key, val));
}

WriteResult NvsStore::writeU32(const char* key, uint32_t val) {
  return CommitWrite(handle_, nvs_set_u32(handle_, key, val));
}

WriteResult NvsStore::writeI32(const char* key, int32_t val) {
  return CommitWrite(handle_, nvs_set_i32(handle_, key, val));
}

WriteResult NvsStore::writeU64(const char* key, uint64_t val) {
  return CommitWrite(handle_, nvs_set_u64(handle_, key, val));
}

WriteResult NvsStore::writeI64(const char* key, int64_t val) {
  return CommitWrite(handle_, nvs_set_i64(handle_, key, val));
}

WriteResult NvsStore::writeFloat(const char* key, float val) {
  return writeBytes(key, &val, sizeof(val));
}

WriteResult NvsStore::writeDouble(const char* key, double val) {
  return writeBytes(key, &val, sizeof(val));
}

WriteResult NvsStore::writeString(const char* key, roo::string_view val) {
  if (!open_ || key == nullptr) return WriteResult::kError;
  if (val.empty()) {
    return CommitWrite(handle_, nvs_set_str(handle_, key, ""));
  }
  // Preserve the on-flash format used by the previous backend: non-empty
  // strings are blobs without a terminating NUL, while empty strings are NVS
  // strings because Arduino Preferences rejected zero-length blobs.
  return writeBytes(key, val.data(), val.size());
}

ReadResult NvsStore::readObjectInternal(const char* key, void* val,
                                        size_t size) {
  size_t stored_size = 0;
  ReadResult status = readBytesLength(key, &stored_size);
  if (status != ReadResult::kOk) return status;
  if (stored_size != size) return ReadResult::kWrongType;
  return readBytes(key, val, size, nullptr);
}

ReadResult NvsStore::readBool(const char* key, bool& val) {
  uint8_t result;
  ReadResult status = readU8(key, result);
  if (status == ReadResult::kOk) val = result == 1;
  return status;
}

ReadResult NvsStore::readU8(const char* key, uint8_t& val) {
  return ReadScalar(nvs_get_u8, handle_, key, val);
}

ReadResult NvsStore::readI8(const char* key, int8_t& val) {
  return ReadScalar(nvs_get_i8, handle_, key, val);
}

ReadResult NvsStore::readU16(const char* key, uint16_t& val) {
  return ReadScalar(nvs_get_u16, handle_, key, val);
}

ReadResult NvsStore::readI16(const char* key, int16_t& val) {
  return ReadScalar(nvs_get_i16, handle_, key, val);
}

ReadResult NvsStore::readU32(const char* key, uint32_t& val) {
  return ReadScalar(nvs_get_u32, handle_, key, val);
}

ReadResult NvsStore::readI32(const char* key, int32_t& val) {
  return ReadScalar(nvs_get_i32, handle_, key, val);
}

ReadResult NvsStore::readU64(const char* key, uint64_t& val) {
  return ReadScalar(nvs_get_u64, handle_, key, val);
}

ReadResult NvsStore::readI64(const char* key, int64_t& val) {
  return ReadScalar(nvs_get_i64, handle_, key, val);
}

ReadResult NvsStore::readFloat(const char* key, float& val) {
  return readObjectInternal(key, &val, sizeof(val));
}

ReadResult NvsStore::readDouble(const char* key, double& val) {
  return readObjectInternal(key, &val, sizeof(val));
}

ReadResult NvsStore::readString(const char* key, std::string& val) {
  size_t size = 0;
  esp_err_t result = nvs_get_str(handle_, key, nullptr, &size);
  if (result == ESP_OK) {
    if (size == 0) return ReadResult::kError;
    std::unique_ptr<char[]> buf(new char[size]);
    result = nvs_get_str(handle_, key, buf.get(), &size);
    if (result != ESP_OK || size == 0 || buf[size - 1] != '\0') {
      return ReadResult::kError;
    }
    val.assign(buf.get(), size - 1);
    return ReadResult::kOk;
  }
  if (result != ESP_ERR_NVS_TYPE_MISMATCH) return ToReadResult(result);

  result = nvs_get_blob(handle_, key, nullptr, &size);
  if (result != ESP_OK) return ToReadResult(result);
  if (size == 0) {
    val.clear();
    return ReadResult::kOk;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  result = nvs_get_blob(handle_, key, buf.get(), &size);
  if (result != ESP_OK) return ToReadResult(result);
  val.assign(buf.get(), size);
  return ReadResult::kOk;
}

ReadResult NvsStore::readBytes(const char* key, void* val, size_t max_len,
                               size_t* out_len) {
  size_t size = 0;
  esp_err_t result = nvs_get_blob(handle_, key, nullptr, &size);
  if (result != ESP_OK) return ToReadResult(result);
  if (out_len != nullptr) *out_len = size;
  if (size == 0) return ReadResult::kOk;
  if (size > max_len || val == nullptr) return ReadResult::kError;
  result = nvs_get_blob(handle_, key, val, &size);
  return ToReadResult(result);
}

ReadResult NvsStore::readBytesLength(const char* key, size_t* out_len) {
  size_t size = 0;
  esp_err_t result = nvs_get_blob(handle_, key, nullptr, &size);
  if (result != ESP_OK) return ToReadResult(result);
  if (size == 0) return ReadResult::kError;
  if (out_len != nullptr) *out_len = size;
  return ReadResult::kOk;
}

}  // namespace roo_prefs

#endif  // ROO_PREFS_USE_ESP32_NVS
