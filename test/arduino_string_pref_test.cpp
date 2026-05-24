#include "gtest/gtest.h"
#include "roo_prefs.h"

namespace roo_prefs {

const char* test_string =
    "Test string, long enough to not fit in the internal stack buffer.";

TEST(PrefsTest, ArduinoString) {
  Collection col("foo");

  ArduinoString pref_writer(col, "arduino_str");
  EXPECT_FALSE(pref_writer.isSet());
  EXPECT_STREQ("", pref_writer.get().c_str());

  EXPECT_TRUE(pref_writer.set(::String(test_string)));

  ArduinoString pref_reader(col, "arduino_str");
  EXPECT_TRUE(pref_reader.isSet());
  EXPECT_STREQ(test_string, pref_reader.get().c_str());

  EXPECT_TRUE(pref_reader.clear());

  ArduinoString pref_cleared(col, "arduino_str");
  EXPECT_FALSE(pref_cleared.isSet());
  EXPECT_STREQ("", pref_cleared.get().c_str());
}

TEST(PrefsTest, StringFromArduinoString) {
  Collection col("foo");

  roo_prefs::String pref_writer(col, "str");
  EXPECT_FALSE(pref_writer.isSet());
  EXPECT_EQ("", pref_writer.get());

  EXPECT_TRUE(pref_writer.set(::String(test_string)));

  roo_prefs::String pref_reader(col, "str");
  EXPECT_TRUE(pref_reader.isSet());
  EXPECT_EQ(test_string, pref_reader.get());

  EXPECT_TRUE(pref_reader.clear());

  roo_prefs::String pref_cleared(col, "str");
  EXPECT_FALSE(pref_cleared.isSet());
  EXPECT_EQ("", pref_cleared.get());
}

}  // namespace roo_prefs