#include <cstring>
#include <new>

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
  EXPECT_EQ(42u, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  roo_time::Delay(roo_time::Millis(500));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(42u, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  lazy.set(84);
  scheduler.executeEligibleTasks();
  EXPECT_EQ(84u, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("u32", val));
  }

  roo_time::Delay(roo_time::Millis(2100));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(84u, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    ASSERT_EQ(ReadResult::kOk, t.store().readU32("u32", val));
    EXPECT_EQ(84u, val);
  }
}

TEST(LazyWritePrefTest, ClearCancelsPendingWrite) {
  system_time_set_auto_sync(false);
  Collection col("lazy_clear");
  roo_scheduler::Scheduler scheduler;
  LazyUint32 lazy(col, scheduler, "clear_key");

  ASSERT_TRUE(lazy.set(42));
  EXPECT_TRUE(lazy.isSet());
  EXPECT_EQ(42u, lazy.get());

  EXPECT_TRUE(lazy.clear());
  EXPECT_FALSE(lazy.isSet());
  EXPECT_EQ(0u, lazy.get());

  roo_time::Delay(roo_time::Millis(2100));
  scheduler.executeEligibleTasks();

  EXPECT_FALSE(lazy.isSet());
  EXPECT_EQ(0u, lazy.get());
  {
    Transaction t(col);
    uint32_t val;
    EXPECT_EQ(ReadResult::kNotFound, t.store().readU32("clear_key", val));
  }
}

TEST(LazyWritePrefTest, ConstructionDoesNotTouchStorage) {
  system_time_set_auto_sync(false);
  Collection col("lazy_ctor");
  roo_scheduler::Scheduler scheduler;
  LazyUint32 lazy(col, scheduler, "ctor_key");

  {
    Transaction t(col);
    ASSERT_EQ(WriteResult::kOk, t.store().writeU32("ctor_key", 123u));
  }

  EXPECT_TRUE(lazy.isSet());
  EXPECT_EQ(123u, lazy.get());
}

TEST(LazyWritePrefTest, FirstSetDoesNotDependOnPoisonedStorage) {
  system_time_set_auto_sync(false);
  Collection col("lazy_poison");
  roo_scheduler::Scheduler scheduler;
  constexpr uint32_t kPoisonValue = 0xA5A5A5A5u;

  alignas(LazyUint32) unsigned char storage[sizeof(LazyUint32)];
  std::memset(storage, 0xA5, sizeof(storage));
  LazyUint32* lazy = new (storage) LazyUint32(col, scheduler, "poison_key");

  EXPECT_TRUE(lazy->set(kPoisonValue));
  EXPECT_TRUE(lazy->isSet());
  EXPECT_EQ(kPoisonValue, lazy->get());

  roo_time::Delay(roo_time::Millis(2100));
  scheduler.executeEligibleTasks();

  {
    Transaction t(col);
    uint32_t val;
    ASSERT_EQ(ReadResult::kOk, t.store().readU32("poison_key", val));
    EXPECT_EQ(kPoisonValue, val);
  }

  lazy->~LazyWritePref<uint32_t>();
}

}  // namespace roo_prefs
