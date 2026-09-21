#include "roo_prefs/store/filesystem_store.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "fakefs_reference.h"
#include "gtest/gtest.h"
#include "roo_prefs.h"

namespace roo_prefs {
namespace {

class FilesystemStoreTest : public testing::Test {
 protected:
  roo_io::fakefs::FakeFs data_;
  roo_io::fakefs::FakeReferenceFs filesystem_{data_};
  FilesystemStore store_{filesystem_};
};

TEST_F(FilesystemStoreTest, RoundTripsAllSupportedTypes) {
  Collection collection("settings", store_);
  Transaction transaction(collection);
  ASSERT_TRUE(transaction.active());
  Store& store = transaction.store();

  EXPECT_EQ(WriteResult::kOk, store.writeBool("bool", true));
  EXPECT_EQ(WriteResult::kOk, store.writeU8("u8", 0xAB));
  EXPECT_EQ(WriteResult::kOk, store.writeI8("i8", -42));
  EXPECT_EQ(WriteResult::kOk, store.writeU16("u16", 0xBEEF));
  EXPECT_EQ(WriteResult::kOk, store.writeI16("i16", -12345));
  EXPECT_EQ(WriteResult::kOk, store.writeU32("u32", 0xDEADBEEFu));
  EXPECT_EQ(WriteResult::kOk, store.writeI32("i32", -123456789));
  EXPECT_EQ(WriteResult::kOk, store.writeU64("u64", 0x0123456789ABCDEFULL));
  EXPECT_EQ(WriteResult::kOk,
            store.writeI64("i64", -INT64_C(1234567890123456789)));
  EXPECT_EQ(WriteResult::kOk, store.writeFloat("float", 3.25f));
  EXPECT_EQ(WriteResult::kOk, store.writeDouble("double", -17.5));
  EXPECT_EQ(WriteResult::kOk, store.writeString("string", "hello"));
  EXPECT_EQ(WriteResult::kOk, store.writeString("empty", ""));
  std::array<uint8_t, 4> blob = {0, 1, 2, 255};
  EXPECT_EQ(WriteResult::kOk,
            store.writeBytes("blob", blob.data(), blob.size()));

  bool boolean = false;
  uint8_t u8 = 0;
  int8_t i8 = 0;
  uint16_t u16 = 0;
  int16_t i16 = 0;
  uint32_t u32 = 0;
  int32_t i32 = 0;
  uint64_t u64 = 0;
  int64_t i64 = 0;
  float float_value = 0;
  double double_value = 0;
  std::string string;
  std::string empty = "not empty";
  std::array<uint8_t, 4> blob_result = {};
  size_t blob_size = 0;

  EXPECT_EQ(ReadResult::kOk, store.readBool("bool", boolean));
  EXPECT_TRUE(boolean);
  EXPECT_EQ(ReadResult::kOk, store.readU8("u8", u8));
  EXPECT_EQ(0xAB, u8);
  EXPECT_EQ(ReadResult::kOk, store.readI8("i8", i8));
  EXPECT_EQ(-42, i8);
  EXPECT_EQ(ReadResult::kOk, store.readU16("u16", u16));
  EXPECT_EQ(0xBEEF, u16);
  EXPECT_EQ(ReadResult::kOk, store.readI16("i16", i16));
  EXPECT_EQ(-12345, i16);
  EXPECT_EQ(ReadResult::kOk, store.readU32("u32", u32));
  EXPECT_EQ(0xDEADBEEFu, u32);
  EXPECT_EQ(ReadResult::kOk, store.readI32("i32", i32));
  EXPECT_EQ(-123456789, i32);
  EXPECT_EQ(ReadResult::kOk, store.readU64("u64", u64));
  EXPECT_EQ(0x0123456789ABCDEFULL, u64);
  EXPECT_EQ(ReadResult::kOk, store.readI64("i64", i64));
  EXPECT_EQ(-INT64_C(1234567890123456789), i64);
  EXPECT_EQ(ReadResult::kOk, store.readFloat("float", float_value));
  EXPECT_FLOAT_EQ(3.25f, float_value);
  EXPECT_EQ(ReadResult::kOk, store.readDouble("double", double_value));
  EXPECT_DOUBLE_EQ(-17.5, double_value);
  EXPECT_EQ(ReadResult::kOk, store.readString("string", string));
  EXPECT_EQ("hello", string);
  EXPECT_EQ(ReadResult::kOk, store.readString("empty", empty));
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(ReadResult::kOk, store.readBytes("blob", blob_result.data(),
                                             blob_result.size(), &blob_size));
  EXPECT_EQ(blob.size(), blob_size);
  EXPECT_EQ(blob, blob_result);
}

TEST_F(FilesystemStoreTest, IntegratesWithTypedPreferences) {
  Collection collection("network", store_);
  Int32 port(collection, "port", 80);
  String host(collection, "host", "localhost");

  EXPECT_FALSE(port.isSet());
  EXPECT_EQ(80, port.get());
  EXPECT_TRUE(port.set(8080));
  EXPECT_TRUE(port.set(8443));
  EXPECT_TRUE(host.set("example.test"));

  FilesystemStore second_store(filesystem_);
  Collection second_collection("network", second_store);
  Int32 stored_port(second_collection, "port");
  String stored_host(second_collection, "host");
  EXPECT_EQ(8443, stored_port.get());
  EXPECT_EQ("example.test", stored_host.get());
}

TEST_F(FilesystemStoreTest, ReportsWrongTypeAndCorruption) {
  Collection collection("typed", store_);
  {
    Transaction transaction(collection);
    ASSERT_TRUE(transaction.active());
    ASSERT_EQ(WriteResult::kOk, transaction.store().writeU32("value", 42));
  }
  {
    Transaction transaction(collection, Transaction::Mode::kReadOnly);
    ASSERT_TRUE(transaction.active());
    int32_t value = 0;
    EXPECT_EQ(ReadResult::kWrongType,
              transaction.store().readI32("value", value));
  }

  roo_io::fakefs::FileStream file = data_.open("/prefs/7479706564/k76616c7565",
                                               roo_io::fakefs::FakeFs::kWrite);
  const roo_io::byte corrupt = static_cast<roo_io::byte>(0);
  file.seek(0);
  ASSERT_EQ(1u, file.write(&corrupt, 1));
  file.close();

  Transaction transaction(collection, Transaction::Mode::kReadOnly);
  ASSERT_TRUE(transaction.active());
  uint32_t value = 0;
  EXPECT_EQ(ReadResult::kError, transaction.store().readU32("value", value));
}

TEST_F(FilesystemStoreTest, EnumeratesAndClearsEncodedKeys) {
  Collection collection("collection/with/slashes", store_);
  Uint8 first(collection, "a/b");
  Uint8 second(collection, "space key");
  ASSERT_TRUE(first.set(1));
  ASSERT_TRUE(second.set(2));

  std::vector<std::string> keys;
  EXPECT_EQ(EnumerateResult::kOk,
            collection.forEachKey([&](roo::string_view key) {
              keys.emplace_back(key.data(), key.size());
              return true;
            }));
  std::sort(keys.begin(), keys.end());
  EXPECT_EQ((std::vector<std::string>{"a/b", "space key"}), keys);

  EXPECT_TRUE(first.clear());
  EXPECT_FALSE(first.isSet());
  Uint8 reloaded(collection, "a/b");
  EXPECT_FALSE(reloaded.isSet());
}

TEST_F(FilesystemStoreTest, RejectsWritesOnReadOnlyMount) {
  filesystem_.setMountingPolicy(roo_io::Filesystem::kMountReadOnly);
  Collection collection("readonly", store_);
  Transaction transaction(collection);
  EXPECT_FALSE(transaction.active());
  EXPECT_EQ(Store::BeginResult::kError, transaction.beginResult());
}

}  // namespace
}  // namespace roo_prefs
