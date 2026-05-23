#include "gtest/gtest.h"
#include "roo_prefs.h"

namespace roo_prefs {

TEST(PrefsTest, ArduinoString) {
  Collection col("foo");

  Pref<::String> pref_writer(col, "arduino_str");
  EXPECT_FALSE(pref_writer.isSet());
  EXPECT_STREQ("", pref_writer.get().c_str());

  EXPECT_TRUE(pref_writer.set(::String("Test string")));

  Pref<::String> pref_reader(col, "arduino_str");
  EXPECT_TRUE(pref_reader.isSet());
  EXPECT_STREQ("Test string", pref_reader.get().c_str());

  EXPECT_TRUE(pref_reader.clear());

  Pref<::String> pref_cleared(col, "arduino_str");
  EXPECT_FALSE(pref_cleared.isSet());
  EXPECT_STREQ("", pref_cleared.get().c_str());
}

}  // namespace roo_prefs