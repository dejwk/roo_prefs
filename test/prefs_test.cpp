#include "gtest/gtest.h"
#include "roo_prefs/pref.h"

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

}  // namespace roo_prefs
