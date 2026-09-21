#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "roo_io/fs/filesystem.h"
#include "roo_prefs/store/store.h"

namespace roo_prefs {

/// Preferences store backed by files in a roo_io filesystem.
///
/// Each preference is stored in a separate, checksummed file. Updates are
/// written to a staging file and installed with Mount::rename(). Power-loss
/// atomicity therefore depends on the selected filesystem backend providing
/// atomic replacement semantics.
class FilesystemStore : public Store {
 public:
  explicit FilesystemStore(roo_io::Filesystem& filesystem,
                           const char* root_path = "/prefs");
  ~FilesystemStore() override;

  FilesystemStore(const FilesystemStore&) = delete;
  FilesystemStore& operator=(const FilesystemStore&) = delete;

  bool isKey(const char* key) override;
  ClearResult clear(const char* key) override;

  WriteResult writeBool(const char* key, bool val) override;
  WriteResult writeU8(const char* key, uint8_t val) override;
  WriteResult writeI8(const char* key, int8_t val) override;
  WriteResult writeU16(const char* key, uint16_t val) override;
  WriteResult writeI16(const char* key, int16_t val) override;
  WriteResult writeU32(const char* key, uint32_t val) override;
  WriteResult writeI32(const char* key, int32_t val) override;
  WriteResult writeU64(const char* key, uint64_t val) override;
  WriteResult writeI64(const char* key, int64_t val) override;
  WriteResult writeFloat(const char* key, float val) override;
  WriteResult writeDouble(const char* key, double val) override;
  WriteResult writeString(const char* key, roo::string_view val) override;
  WriteResult writeBytes(const char* key, const void* val, size_t len) override;

  ReadResult readBool(const char* key, bool& val) override;
  ReadResult readU8(const char* key, uint8_t& val) override;
  ReadResult readI8(const char* key, int8_t& val) override;
  ReadResult readU16(const char* key, uint16_t& val) override;
  ReadResult readI16(const char* key, int16_t& val) override;
  ReadResult readU32(const char* key, uint32_t& val) override;
  ReadResult readI32(const char* key, int32_t& val) override;
  ReadResult readU64(const char* key, uint64_t& val) override;
  ReadResult readI64(const char* key, int64_t& val) override;
  ReadResult readFloat(const char* key, float& val) override;
  ReadResult readDouble(const char* key, double& val) override;
  ReadResult readString(const char* key, std::string& val) override;
  ReadResult readBytes(const char* key, void* val, size_t max_len,
                       size_t* out_len) override;
  ReadResult readBytesLength(const char* key, size_t* out_len) override;

 private:
  enum class ValueType : uint8_t {
    kBool = 1,
    kU8,
    kI8,
    kU16,
    kI16,
    kU32,
    kI32,
    kU64,
    kI64,
    kFloat,
    kDouble,
    kString,
    kBlob,
  };

  BeginResult begin(const char* collection_name, bool read_only) override;
  void end() override;
  EnumerateResult enumerateKeys(const char* collection_name, KeyVisitor visitor,
                                void* context) const override;
  WriteResult writeObjectInternal(const char* key, const void* val,
                                  size_t size) override;
  ReadResult readObjectInternal(const char* key, void* val,
                                size_t size) override;

  WriteResult writeValue(const char* key, ValueType type, const void* value,
                         size_t size);
  ReadResult readValue(const char* key, ValueType type,
                       std::vector<uint8_t>& value);

  template <typename UInt>
  WriteResult writeUnsigned(const char* key, ValueType type, UInt value);

  template <typename UInt>
  ReadResult readUnsigned(const char* key, ValueType type, UInt& value);

  static std::string normalizeRoot(const char* root_path);
  static std::string encodeName(const char* name);
  static bool decodeName(roo::string_view encoded, std::string& name);
  static roo_io::Status ensureDirectory(roo_io::Mount& mount,
                                        const std::string& path);

  std::string collectionPath(const char* collection_name) const;
  std::string keyPath(const char* key) const;

  roo_io::Filesystem& filesystem_;
  std::string root_path_;
  roo_io::Mount mount_;
  std::string collection_path_;
  bool open_;
  bool read_only_;
};

}  // namespace roo_prefs

#include "roo_prefs/store/filesystem_store.ipp"
