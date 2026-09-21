#pragma once

#include <cstring>
#include <limits>
#include <memory>
#include <utility>

#include "roo_io/base/byte.h"
#include "roo_io/fs/file_update_policy.h"

namespace roo_prefs {
namespace filesystem_store_internal {

constexpr size_t kHeaderSize = 16;
constexpr uint8_t kFormatVersion = 1;
constexpr uint8_t kMagic[] = {'R', 'P', 'F', 'S'};

inline uint32_t Crc32Update(uint32_t crc, const uint8_t* data, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
  }
  return crc;
}

inline uint32_t LoadU32(const uint8_t* value) {
  return static_cast<uint32_t>(value[0]) |
         (static_cast<uint32_t>(value[1]) << 8) |
         (static_cast<uint32_t>(value[2]) << 16) |
         (static_cast<uint32_t>(value[3]) << 24);
}

inline void StoreU32(uint8_t* target, uint32_t value) {
  target[0] = static_cast<uint8_t>(value);
  target[1] = static_cast<uint8_t>(value >> 8);
  target[2] = static_cast<uint8_t>(value >> 16);
  target[3] = static_cast<uint8_t>(value >> 24);
}

inline bool IsHex(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

inline uint8_t FromHex(char c) {
  return c <= '9' ? static_cast<uint8_t>(c - '0')
                  : static_cast<uint8_t>(c - 'a' + 10);
}

}  // namespace filesystem_store_internal

inline FilesystemStore::FilesystemStore(roo_io::Filesystem& filesystem,
                                        const char* root_path)
    : filesystem_(filesystem),
      root_path_(normalizeRoot(root_path)),
      mount_(),
      open_(false),
      read_only_(true) {}

inline FilesystemStore::~FilesystemStore() { end(); }

inline std::string FilesystemStore::normalizeRoot(const char* root_path) {
  if (root_path == nullptr || root_path[0] != '/') return std::string();
  std::string result(root_path);
  while (result.size() > 1 && result.back() == '/') result.pop_back();
  return result;
}

inline std::string FilesystemStore::encodeName(const char* name) {
  static constexpr char kHex[] = "0123456789abcdef";
  if (name == nullptr || name[0] == '\0') return std::string();
  std::string result;
  result.reserve(std::strlen(name) * 2);
  for (const uint8_t* p = reinterpret_cast<const uint8_t*>(name); *p; ++p) {
    result.push_back(kHex[*p >> 4]);
    result.push_back(kHex[*p & 0x0F]);
  }
  return result;
}

inline bool FilesystemStore::decodeName(roo::string_view encoded,
                                        std::string& name) {
  using namespace filesystem_store_internal;
  if (encoded.empty() || encoded.size() % 2 != 0) return false;
  name.clear();
  name.reserve(encoded.size() / 2);
  for (size_t i = 0; i < encoded.size(); i += 2) {
    if (!IsHex(encoded[i]) || !IsHex(encoded[i + 1])) return false;
    uint8_t value = static_cast<uint8_t>((FromHex(encoded[i]) << 4) |
                                         FromHex(encoded[i + 1]));
    if (value == 0) return false;
    name.push_back(static_cast<char>(value));
  }
  return true;
}

inline roo_io::Status FilesystemStore::ensureDirectory(
    roo_io::Mount& mount, const std::string& path) {
  if (path.empty() || path[0] != '/') return roo_io::kInvalidPath;
  if (path == "/") return roo_io::kOk;
  std::string partial;
  for (size_t pos = 1; pos <= path.size(); ++pos) {
    if (pos != path.size() && path[pos] != '/') continue;
    if (pos == 1) continue;
    partial.assign(path.data(), pos);
    roo_io::Stat stat = mount.stat(partial.c_str());
    if (stat.isDirectory()) continue;
    if (stat.status() != roo_io::kNotFound) {
      return stat.exists() ? roo_io::kNotDirectory : stat.status();
    }
    roo_io::Status status = mount.mkdir(partial.c_str());
    if (status != roo_io::kOk && status != roo_io::kDirectoryExists) {
      return status;
    }
  }
  return roo_io::kOk;
}

inline std::string FilesystemStore::collectionPath(
    const char* collection_name) const {
  std::string encoded = encodeName(collection_name);
  if (root_path_.empty() || encoded.empty()) return std::string();
  return root_path_ == "/" ? "/" + encoded : root_path_ + "/" + encoded;
}

inline std::string FilesystemStore::keyPath(const char* key) const {
  std::string encoded = encodeName(key);
  if (!open_ || encoded.empty()) return std::string();
  return collection_path_ + "/k" + encoded;
}

inline Store::BeginResult FilesystemStore::begin(const char* collection_name,
                                                 bool read_only) {
  if (open_) return BeginResult::kError;
  std::string path = collectionPath(collection_name);
  if (path.empty()) return BeginResult::kError;

  roo_io::Mount mount = filesystem_.mount();
  if (!mount.ok() || (!read_only && mount.isReadOnly())) {
    return BeginResult::kError;
  }
  roo_io::Stat stat = mount.stat(path.c_str());
  if (stat.status() == roo_io::kNotFound) {
    if (read_only) return BeginResult::kNotFound;
    if (ensureDirectory(mount, path) != roo_io::kOk) {
      return BeginResult::kError;
    }
  } else if (!stat.isDirectory()) {
    return BeginResult::kError;
  }

  mount_ = std::move(mount);
  collection_path_ = std::move(path);
  open_ = true;
  read_only_ = read_only;
  return BeginResult::kOk;
}

inline void FilesystemStore::end() {
  if (!open_) return;
  mount_.close();
  collection_path_.clear();
  open_ = false;
  read_only_ = true;
}

inline bool FilesystemStore::isKey(const char* key) {
  std::string path = keyPath(key);
  return !path.empty() && mount_.stat(path.c_str()).isFile();
}

inline ClearResult FilesystemStore::clear(const char* key) {
  if (!open_ || read_only_) return ClearResult::kError;
  std::string path = keyPath(key);
  if (path.empty()) return ClearResult::kError;
  return mount_.remove(path.c_str()) == roo_io::kOk ? ClearResult::kOk
                                                    : ClearResult::kError;
}

inline WriteResult FilesystemStore::writeValue(const char* key, ValueType type,
                                               const void* value, size_t size) {
  using namespace filesystem_store_internal;
  if (!open_ || read_only_ || (value == nullptr && size != 0) ||
      size > std::numeric_limits<uint32_t>::max()) {
    return WriteResult::kError;
  }
  std::string path = keyPath(key);
  if (path.empty()) return WriteResult::kError;

  std::vector<uint8_t> existing;
  if (readValue(key, type, existing) == ReadResult::kOk &&
      existing.size() == size &&
      (size == 0 || std::memcmp(existing.data(), value, size) == 0)) {
    return WriteResult::kOk;
  }

  uint8_t header[kHeaderSize] = {};
  std::memcpy(header, kMagic, sizeof(kMagic));
  header[4] = kFormatVersion;
  header[5] = static_cast<uint8_t>(type);
  StoreU32(header + 8, static_cast<uint32_t>(size));
  uint32_t crc = Crc32Update(0xFFFFFFFFu, header, 12);
  crc =
      Crc32Update(crc, static_cast<const uint8_t*>(value), size) ^ 0xFFFFFFFFu;
  StoreU32(header + 12, crc);

  std::string staging = path + ".tmp";
  std::unique_ptr<roo_io::OutputStream> output =
      mount_.fopenForWrite(staging.c_str(), roo_io::kTruncateIfExists);
  if (!output || !output->isOpen() ||
      output->writeFully(reinterpret_cast<const roo_io::byte*>(header),
                         sizeof(header)) != sizeof(header) ||
      (size != 0 && output->writeFully(static_cast<const roo_io::byte*>(value),
                                       size) != size) ||
      output->status() != roo_io::kOk) {
    output.reset();
    mount_.remove(staging.c_str());
    return WriteResult::kError;
  }
  output.reset();
  if (mount_.rename(staging.c_str(), path.c_str()) != roo_io::kOk) {
    mount_.remove(staging.c_str());
    return WriteResult::kError;
  }
  return WriteResult::kOk;
}

inline ReadResult FilesystemStore::readValue(const char* key, ValueType type,
                                             std::vector<uint8_t>& value) {
  using namespace filesystem_store_internal;
  std::string path = keyPath(key);
  if (path.empty()) return ReadResult::kError;
  std::unique_ptr<roo_io::MultipassInputStream> input =
      mount_.fopen(path.c_str());
  if (!input || input->status() == roo_io::kNotFound) {
    return ReadResult::kNotFound;
  }
  if (!input->isOpen()) return ReadResult::kError;

  uint8_t header[kHeaderSize];
  if (input->size() < kHeaderSize ||
      input->readFully(reinterpret_cast<roo_io::byte*>(header),
                       sizeof(header)) != sizeof(header) ||
      std::memcmp(header, kMagic, sizeof(kMagic)) != 0 ||
      header[4] != kFormatVersion || header[6] != 0 || header[7] != 0) {
    return ReadResult::kError;
  }
  if (header[5] != static_cast<uint8_t>(type)) {
    return ReadResult::kWrongType;
  }
  uint32_t size = LoadU32(header + 8);
  if (input->size() != kHeaderSize + static_cast<uint64_t>(size)) {
    return ReadResult::kError;
  }
  value.resize(size);
  if (size != 0 &&
      input->readFully(reinterpret_cast<roo_io::byte*>(value.data()), size) !=
          size) {
    return ReadResult::kError;
  }
  uint32_t crc = Crc32Update(0xFFFFFFFFu, header, 12);
  crc = Crc32Update(crc, value.data(), value.size()) ^ 0xFFFFFFFFu;
  return crc == LoadU32(header + 12) ? ReadResult::kOk : ReadResult::kError;
}

template <typename UInt>
inline WriteResult FilesystemStore::writeUnsigned(const char* key,
                                                  ValueType type, UInt value) {
  uint8_t bytes[sizeof(UInt)];
  for (size_t i = 0; i < sizeof(UInt); ++i) {
    bytes[i] = static_cast<uint8_t>(value >> (i * 8));
  }
  return writeValue(key, type, bytes, sizeof(bytes));
}

template <typename UInt>
inline ReadResult FilesystemStore::readUnsigned(const char* key, ValueType type,
                                                UInt& value) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, type, bytes);
  if (result != ReadResult::kOk) return result;
  if (bytes.size() != sizeof(UInt)) return ReadResult::kError;
  UInt decoded = 0;
  for (size_t i = 0; i < sizeof(UInt); ++i) {
    decoded |= static_cast<UInt>(bytes[i]) << (i * 8);
  }
  value = decoded;
  return ReadResult::kOk;
}

inline WriteResult FilesystemStore::writeBool(const char* key, bool val) {
  uint8_t value = val ? 1 : 0;
  return writeValue(key, ValueType::kBool, &value, sizeof(value));
}

inline WriteResult FilesystemStore::writeU8(const char* key, uint8_t val) {
  return writeUnsigned(key, ValueType::kU8, val);
}

inline WriteResult FilesystemStore::writeI8(const char* key, int8_t val) {
  uint8_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kI8, bits);
}

inline WriteResult FilesystemStore::writeU16(const char* key, uint16_t val) {
  return writeUnsigned(key, ValueType::kU16, val);
}

inline WriteResult FilesystemStore::writeI16(const char* key, int16_t val) {
  uint16_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kI16, bits);
}

inline WriteResult FilesystemStore::writeU32(const char* key, uint32_t val) {
  return writeUnsigned(key, ValueType::kU32, val);
}

inline WriteResult FilesystemStore::writeI32(const char* key, int32_t val) {
  uint32_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kI32, bits);
}

inline WriteResult FilesystemStore::writeU64(const char* key, uint64_t val) {
  return writeUnsigned(key, ValueType::kU64, val);
}

inline WriteResult FilesystemStore::writeI64(const char* key, int64_t val) {
  uint64_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kI64, bits);
}

inline WriteResult FilesystemStore::writeFloat(const char* key, float val) {
  uint32_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kFloat, bits);
}

inline WriteResult FilesystemStore::writeDouble(const char* key, double val) {
  uint64_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  return writeUnsigned(key, ValueType::kDouble, bits);
}

inline WriteResult FilesystemStore::writeString(const char* key,
                                                roo::string_view val) {
  return writeValue(key, ValueType::kString, val.data(), val.size());
}

inline WriteResult FilesystemStore::writeBytes(const char* key, const void* val,
                                               size_t len) {
  if (val == nullptr || len == 0) return WriteResult::kError;
  return writeValue(key, ValueType::kBlob, val, len);
}

inline WriteResult FilesystemStore::writeObjectInternal(const char* key,
                                                        const void* val,
                                                        size_t size) {
  return writeBytes(key, val, size);
}

inline ReadResult FilesystemStore::readBool(const char* key, bool& val) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, ValueType::kBool, bytes);
  if (result != ReadResult::kOk) return result;
  if (bytes.size() != 1 || bytes[0] > 1) return ReadResult::kError;
  val = bytes[0] != 0;
  return ReadResult::kOk;
}

inline ReadResult FilesystemStore::readU8(const char* key, uint8_t& val) {
  return readUnsigned(key, ValueType::kU8, val);
}

inline ReadResult FilesystemStore::readI8(const char* key, int8_t& val) {
  uint8_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kI8, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readU16(const char* key, uint16_t& val) {
  return readUnsigned(key, ValueType::kU16, val);
}

inline ReadResult FilesystemStore::readI16(const char* key, int16_t& val) {
  uint16_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kI16, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readU32(const char* key, uint32_t& val) {
  return readUnsigned(key, ValueType::kU32, val);
}

inline ReadResult FilesystemStore::readI32(const char* key, int32_t& val) {
  uint32_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kI32, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readU64(const char* key, uint64_t& val) {
  return readUnsigned(key, ValueType::kU64, val);
}

inline ReadResult FilesystemStore::readI64(const char* key, int64_t& val) {
  uint64_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kI64, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readFloat(const char* key, float& val) {
  uint32_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kFloat, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readDouble(const char* key, double& val) {
  uint64_t bits = 0;
  ReadResult result = readUnsigned(key, ValueType::kDouble, bits);
  if (result == ReadResult::kOk) std::memcpy(&val, &bits, sizeof(val));
  return result;
}

inline ReadResult FilesystemStore::readString(const char* key,
                                              std::string& val) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, ValueType::kString, bytes);
  if (result == ReadResult::kOk) {
    if (bytes.empty()) {
      val.clear();
    } else {
      val.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
  }
  return result;
}

inline ReadResult FilesystemStore::readBytes(const char* key, void* val,
                                             size_t max_len, size_t* out_len) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, ValueType::kBlob, bytes);
  if (result != ReadResult::kOk) return result;
  if (out_len != nullptr) *out_len = bytes.size();
  if (bytes.size() > max_len || (bytes.size() != 0 && val == nullptr)) {
    return ReadResult::kError;
  }
  if (!bytes.empty()) std::memcpy(val, bytes.data(), bytes.size());
  return ReadResult::kOk;
}

inline ReadResult FilesystemStore::readBytesLength(const char* key,
                                                   size_t* out_len) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, ValueType::kBlob, bytes);
  if (result != ReadResult::kOk) return result;
  if (bytes.empty()) return ReadResult::kError;
  if (out_len != nullptr) *out_len = bytes.size();
  return ReadResult::kOk;
}

inline ReadResult FilesystemStore::readObjectInternal(const char* key,
                                                      void* val, size_t size) {
  std::vector<uint8_t> bytes;
  ReadResult result = readValue(key, ValueType::kBlob, bytes);
  if (result != ReadResult::kOk) return result;
  if (bytes.size() != size) return ReadResult::kWrongType;
  if (size != 0) std::memcpy(val, bytes.data(), size);
  return ReadResult::kOk;
}

inline EnumerateResult FilesystemStore::enumerateKeys(
    const char* collection_name, KeyVisitor visitor, void* context) const {
  if (visitor == nullptr) return EnumerateResult::kError;
  std::string path = collectionPath(collection_name);
  if (path.empty()) return EnumerateResult::kError;
  roo_io::Mount mount = filesystem_.mount();
  if (!mount.ok()) return EnumerateResult::kError;
  roo_io::Stat stat = mount.stat(path.c_str());
  if (stat.status() == roo_io::kNotFound) return EnumerateResult::kOk;
  if (!stat.isDirectory()) return EnumerateResult::kError;
  roo_io::Directory directory = mount.opendir(path.c_str());
  if (!directory.isOpen()) return EnumerateResult::kError;
  while (directory.read()) {
    const roo_io::Directory::Entry& entry = directory.entry();
    roo::string_view filename(entry.name());
    if (entry.isDirectory() || filename.size() < 3 || filename[0] != 'k') {
      continue;
    }
    std::string key;
    if (!decodeName(filename.substr(1), key)) continue;
    if (!visitor(context, roo::string_view(key))) {
      return EnumerateResult::kStopped;
    }
  }
  return directory.status() == roo_io::kEndOfStream ? EnumerateResult::kOk
                                                    : EnumerateResult::kError;
}

}  // namespace roo_prefs
