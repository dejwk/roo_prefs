#include "roo_prefs/lazy_write_pref.h"

#include "gtest/gtest.h"
#include "roo_testing/system/timer.h"

namespace roo_prefs {
TEST(LazyWritePrefTest, BasicOperations) {
  system_time_set_auto_sync(false);
  Collection col("foo");
  roo_scheduler::Scheduler scheduler;
  LazyUint32 lazy(col, scheduler, "u32");

  // To test write-through.
  EXPECT_FALSE(lazy.isSet());
  scheduler.executeEligibleTasks();
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  lazy.set(42);
  scheduler.executeEligibleTasks();
  EXPECT_EQ(42, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  roo_time::Delay(roo_time::Millis(500));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(42, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  lazy.set(84);
  scheduler.executeEligibleTasks();
  EXPECT_EQ(84, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  roo_time::Delay(roo_time::Millis(2100));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(84, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    ASSERT_EQ(ReadResult::kOk, t.store().readU32("u32", val));
    EXPECT_EQ(84, val);
  }
}

}  // namespace roo_prefs
