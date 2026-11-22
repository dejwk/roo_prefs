#include "roo_prefs/lazy_write_pref.h"

#include <cstring>
#include <new>

#include "gtest/gtest.h"
#include "roo_testing/system/timer.h"

namespace roo_prefs {
TEST(LazyWritePrefTest, BasicOperations) {
  Collection col("foo");
  roo_scheduler::SchedulingService scheduler;
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

/// Verifies that explicitly setting the default persists an unset key.
TEST(LazyWritePrefTest, PersistsExplicitDefault) {
  Collection col("lazy_default");
  roo_scheduler::SchedulingService scheduler;
  constexpr uint32_t kDefault = 42;
  LazyUint32 lazy(col, scheduler, "value", kDefault);

  EXPECT_FALSE(lazy.isSet());
  ASSERT_TRUE(lazy.set(kDefault));
  EXPECT_TRUE(lazy.isSet());
  EXPECT_FALSE(Uint32(col, "value").isSet());

  roo_time::Delay(roo_time::Seconds(2));
  scheduler.executeEligibleTasks();

  Uint32 saved(col, "value");
  EXPECT_TRUE(saved.isSet());
  EXPECT_EQ(kDefault, saved.get());
}

/// Verifies that a change restarts the stable interval from the change time.
TEST(LazyWritePrefTest, FlushesAtStableDeadlineAfterChange) {
  Collection col("lazy_stable");
  roo_scheduler::SchedulingService scheduler;
  LazyUint32 lazy(col, scheduler, "value", 0, 3, 10);

  ASSERT_TRUE(lazy.set(1));
  roo_time::Delay(roo_time::Millis(2500));
  ASSERT_TRUE(lazy.set(2));
  roo_time::Delay(roo_time::Millis(500));
  scheduler.executeEligibleTasks();

  roo_time::Delay(roo_time::Millis(2499));
  scheduler.executeEligibleTasks();
  EXPECT_FALSE(Uint32(col, "value").isSet());

  roo_time::Delay(roo_time::Millis(1));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(2u, Uint32(col, "value").get());
}

/// Verifies the maximum delay when changes keep restarting the stable interval.
TEST(LazyWritePrefTest, FlushesAtMaximumDeadlineWhileChanging) {
  Collection col("lazy_maximum");
  roo_scheduler::SchedulingService scheduler;
  LazyUint32 lazy(col, scheduler, "value", 0, 3, 5);

  ASSERT_TRUE(lazy.set(1));
  roo_time::Delay(roo_time::Millis(2500));
  ASSERT_TRUE(lazy.set(2));
  roo_time::Delay(roo_time::Millis(500));
  scheduler.executeEligibleTasks();
  roo_time::Delay(roo_time::Millis(1500));
  ASSERT_TRUE(lazy.set(3));

  roo_time::Delay(roo_time::Millis(499));
  scheduler.executeEligibleTasks();
  EXPECT_FALSE(Uint32(col, "value").isSet());

  roo_time::Delay(roo_time::Millis(1));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(3u, Uint32(col, "value").get());
}

/// Verifies that zero-delay writes retry later after a storage failure.
TEST(LazyWritePrefTest, RetriesFailedImmediateWrite) {
  Collection col("lazy_retry");
  roo_scheduler::SchedulingService scheduler;
  LazyUint32 lazy(col, scheduler, "value", 0, 0);

  ASSERT_TRUE(Uint32(col, "value").set(1));
  {
    Transaction read_only(col, Transaction::Mode::kReadOnly);
    ASSERT_TRUE(read_only.active());
    ASSERT_TRUE(lazy.set(2));
    scheduler.executeEligibleTasks(1);
    EXPECT_EQ(2u, lazy.get());
    EXPECT_EQ(1u, Uint32(col, "value").get());
  }

  scheduler.executeEligibleTasks(1);
  EXPECT_EQ(1u, Uint32(col, "value").get());

  roo_time::Delay(roo_time::Seconds(1));
  scheduler.executeEligibleTasks();
  EXPECT_EQ(2u, Uint32(col, "value").get());
}

TEST(LazyWritePrefTest, ClearCancelsPendingWrite) {
  Collection col("lazy_clear");
  roo_scheduler::SchedulingService scheduler;
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
  Collection col("lazy_ctor");
  roo_scheduler::SchedulingService scheduler;
  LazyUint32 lazy(col, scheduler, "ctor_key");

  {
    Transaction t(col);
    ASSERT_EQ(WriteResult::kOk, t.store().writeU32("ctor_key", 123u));
  }

  EXPECT_TRUE(lazy.isSet());
  EXPECT_EQ(123u, lazy.get());
}

TEST(LazyWritePrefTest, FirstSetDoesNotDependOnPoisonedStorage) {
  Collection col("lazy_poison");
  roo_scheduler::SchedulingService scheduler;
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
