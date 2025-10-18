#include "gtest/gtest.h"
#include "roo_prefs.h"

namespace roo_prefs {
TEST(PrefsTest, SampleTest) {
  Collection col("foo");
  roo_prefs::Int16 pref1(col, "pref1");
  roo_prefs::String pref2(col, "pref2");

  EXPECT_EQ(0, pref1.get());
  EXPECT_FALSE(pref1.isSet());
  EXPECT_EQ("", pref2.get());
  EXPECT_FALSE(pref2.isSet());

  // Set the prefs.
  pref1.set(42);
  pref2.set("Hello");
  EXPECT_EQ(42, pref1.get());
  EXPECT_TRUE(pref1.isSet());
  EXPECT_EQ("Hello", pref2.get());
  EXPECT_TRUE(pref2.isSet());

  // Clear the prefs.
  pref1.clear();
  pref2.clear();
  EXPECT_EQ(0, pref1.get());
  EXPECT_FALSE(pref1.isSet());
  EXPECT_EQ("", pref2.get());
  EXPECT_FALSE(pref2.isSet());
}

TEST(PrefsTest, AllSimpleTypes) {
  Collection col("foo");

  roo_prefs::Bool pref_bool(col, "bool");
  roo_prefs::Uint8 pref_u8(col, "u8");
  roo_prefs::Int8 pref_i8(col, "i8");
  roo_prefs::Uint16 pref_u16(col, "u16");
  roo_prefs::Int16 pref_i16(col, "i16");
  roo_prefs::Uint32 pref_u32(col, "u32");
  roo_prefs::Int32 pref_i32(col, "i32");
  roo_prefs::Uint64 pref_u64(col, "u64");
  roo_prefs::Int64 pref_i64(col, "i64");
  roo_prefs::Float pref_float(col, "float");
  roo_prefs::Double pref_double(col, "double");

  EXPECT_FALSE(pref_bool.isSet());
  EXPECT_EQ(false, pref_bool.get());
  pref_bool.set(true);
  EXPECT_TRUE(pref_bool.isSet());
  EXPECT_EQ(true, pref_bool.get());
  pref_bool.clear();
  EXPECT_FALSE(pref_bool.isSet());
  EXPECT_EQ(false, pref_bool.get());

  EXPECT_FALSE(pref_u8.isSet());
  EXPECT_EQ(0, pref_u8.get());
  pref_u8.set(123);
  EXPECT_TRUE(pref_u8.isSet());
  EXPECT_EQ(123, pref_u8.get());
  pref_u8.clear();
  EXPECT_FALSE(pref_u8.isSet());
  EXPECT_EQ(0, pref_u8.get());

  EXPECT_FALSE(pref_i8.isSet());
  EXPECT_EQ(0, pref_i8.get());
  pref_i8.set(-123);
  EXPECT_TRUE(pref_i8.isSet());
  EXPECT_EQ(-123, pref_i8.get());
  pref_i8.clear();
  EXPECT_FALSE(pref_i8.isSet());
  EXPECT_EQ(0, pref_i8.get());

  EXPECT_FALSE(pref_u16.isSet());
  EXPECT_EQ(0, pref_u16.get());
  pref_u16.set(12345);
  EXPECT_TRUE(pref_u16.isSet());
  EXPECT_EQ(12345, pref_u16.get());
  pref_u16.clear();
  EXPECT_FALSE(pref_u16.isSet());
  EXPECT_EQ(0, pref_u16.get());

  EXPECT_FALSE(pref_i16.isSet());
  EXPECT_EQ(0, pref_i16.get());
  pref_i16.set(-12345);
  EXPECT_TRUE(pref_i16.isSet());
  EXPECT_EQ(-12345, pref_i16.get());
  pref_i16.clear();
  EXPECT_FALSE(pref_i16.isSet());
  EXPECT_EQ(0, pref_i16.get());

  EXPECT_FALSE(pref_u32.isSet());
  EXPECT_EQ(0, pref_u32.get());
  pref_u32.set(1234567890);
  EXPECT_TRUE(pref_u32.isSet());
  EXPECT_EQ(1234567890, pref_u32.get());
  pref_u32.clear();
  EXPECT_FALSE(pref_u32.isSet());
  EXPECT_EQ(0, pref_u32.get());

  EXPECT_FALSE(pref_i32.isSet());
  EXPECT_EQ(0, pref_i32.get());
  pref_i32.set(-1234567890);
  EXPECT_TRUE(pref_i32.isSet());
  EXPECT_EQ(-1234567890, pref_i32.get());
  pref_i32.clear();
  EXPECT_FALSE(pref_i32.isSet());
  EXPECT_EQ(0, pref_i32.get());

  EXPECT_FALSE(pref_u64.isSet());
  EXPECT_EQ(0, pref_u64.get());
  pref_u64.set(1234567890123456789ULL);
  EXPECT_TRUE(pref_u64.isSet());
  EXPECT_EQ(1234567890123456789ULL, pref_u64.get());
  pref_u64.clear();
  EXPECT_FALSE(pref_u64.isSet());
  EXPECT_EQ(0, pref_u64.get());

  EXPECT_FALSE(pref_i64.isSet());
  EXPECT_EQ(0, pref_i64.get());
  pref_i64.set(-1234567890123456789LL);
  EXPECT_TRUE(pref_i64.isSet());
  EXPECT_EQ(-1234567890123456789LL, pref_i64.get());
  pref_i64.clear();
  EXPECT_FALSE(pref_i64.isSet());
  EXPECT_EQ(0, pref_i64.get());

  EXPECT_FALSE(pref_float.isSet());
  EXPECT_EQ(0.0f, pref_float.get());
  pref_float.set(3.14159f);
  EXPECT_TRUE(pref_float.isSet());
  EXPECT_FLOAT_EQ(3.14159f, pref_float.get());
  pref_float.clear();
  EXPECT_FALSE(pref_float.isSet());
  EXPECT_EQ(0.0f, pref_float.get());

  EXPECT_FALSE(pref_double.isSet());
  EXPECT_EQ(0.0, pref_double.get());
  pref_double.set(2.718281828459045);
  EXPECT_TRUE(pref_double.isSet());
  EXPECT_DOUBLE_EQ(2.718281828459045, pref_double.get());
  pref_double.clear();
  EXPECT_FALSE(pref_double.isSet());
  EXPECT_EQ(0.0, pref_double.get());
}

TEST(PrefsTest, String) {
  Collection col("foo");
  roo_prefs::String pref_str(col, "str");

  EXPECT_FALSE(pref_str.isSet());
  EXPECT_EQ("", pref_str.get());
  pref_str.set("Test string");
  EXPECT_TRUE(pref_str.isSet());
  EXPECT_EQ("Test string", pref_str.get());
  pref_str.clear();
  EXPECT_FALSE(pref_str.isSet());
  EXPECT_EQ("", pref_str.get());
}

TEST(PrefsTest, Struct) {
  struct MyStruct {
    MyStruct(int32_t a = 0, float b = 0.0f) : a(a), b(b) {}
    bool operator==(const MyStruct& other) const {
      return (a == other.a) && (b == other.b);
    }
    int32_t a;
    float b;
  };

  Collection col("foo");
  roo_prefs::Pref<MyStruct> pref_struct(col, "mystruct");

  EXPECT_FALSE(pref_struct.isSet());
  MyStruct default_struct = pref_struct.get();
  EXPECT_EQ(0, default_struct.a);
  EXPECT_FLOAT_EQ(0.0f, default_struct.b);

  MyStruct s1(42, 3.14f);
  pref_struct.set(s1);
  EXPECT_TRUE(pref_struct.isSet());
  MyStruct s2 = pref_struct.get();
  EXPECT_EQ(42, s2.a);
  EXPECT_FLOAT_EQ(3.14f, s2.b);

  pref_struct.clear();
  EXPECT_FALSE(pref_struct.isSet());
  MyStruct s3 = pref_struct.get();
  EXPECT_EQ(0, s3.a);
  EXPECT_FLOAT_EQ(0.0f, s3.b);
}

TEST(PrefsTest, DefaultValues) {
  Collection col("foo");
  roo_prefs::Int32 pref_int(col, "pref_int", 99);
  roo_prefs::String pref_str(col, "pref_str", "default");

  EXPECT_FALSE(pref_int.isSet());
  EXPECT_EQ(99, pref_int.get());
  pref_int.set(123);
  EXPECT_TRUE(pref_int.isSet());
  EXPECT_EQ(123, pref_int.get());
  pref_int.clear();
  EXPECT_FALSE(pref_int.isSet());
  EXPECT_EQ(99, pref_int.get());

  EXPECT_FALSE(pref_str.isSet());
  EXPECT_EQ("default", pref_str.get());
  pref_str.set("changed");
  EXPECT_TRUE(pref_str.isSet());
  EXPECT_EQ("changed", pref_str.get());
  pref_str.clear();
  EXPECT_FALSE(pref_str.isSet());
  EXPECT_EQ("default", pref_str.get());
}

TEST(PrefsTest, DirectAccess) {
  Collection col("foo");
  {
    EXPECT_FALSE(col.inTransaction());
    Transaction t(col);
    EXPECT_TRUE(col.inTransaction());
    EXPECT_FALSE(t.store().isKey("pref_int"));
    int32_t val = 1234;
    EXPECT_EQ(READ_NOT_FOUND, t.store().readI32("pref_int", val));
    t.store().writeI32("pref_int", 999);
    EXPECT_TRUE(t.store().isKey("pref_int"));
    EXPECT_EQ(READ_OK, t.store().readI32("pref_int", val));
    EXPECT_EQ(val, 999);
    uint32_t uval = 1234;
    EXPECT_EQ(READ_WRONG_TYPE, t.store().readU32("pref_int", uval));
  }
  {
    Transaction t(col);
    EXPECT_TRUE(t.store().isKey("pref_int"));
    int32_t val = 1234;
    EXPECT_EQ(READ_OK, t.store().readI32("pref_int", val));
    EXPECT_EQ(val, 999);
    t.store().clear("pref_int");
    EXPECT_FALSE(t.store().isKey("pref_int"));
    EXPECT_EQ(READ_NOT_FOUND, t.store().readI32("pref_int", val));
  }
  {
    Transaction t(col);
    EXPECT_FALSE(t.store().isKey("pref_int"));
    int32_t val = 1234;
    EXPECT_EQ(READ_NOT_FOUND, t.store().readI32("pref_int", val));
  }
  EXPECT_FALSE(col.inTransaction());
}

TEST(PrefsTest, NestedTransactions) {
  Collection col("foo");
  Uint32 pref(col, "pref");
  EXPECT_FALSE(col.inTransaction());
  {
    Transaction t1(col);
    EXPECT_TRUE(col.inTransaction());
    pref.set(42);
    {
      Transaction t2(col);
      EXPECT_TRUE(col.inTransaction());
      EXPECT_EQ(42, pref.get());
      pref.set(84);
    }
    EXPECT_TRUE(col.inTransaction());
    EXPECT_EQ(84, pref.get());
  }
  EXPECT_FALSE(col.inTransaction());
}

}  // namespace roo_prefs
