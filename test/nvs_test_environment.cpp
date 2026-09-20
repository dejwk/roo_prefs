#include "roo_prefs/store/platform.h"

#if ROO_PREFS_USE_ESP32_NVS

#include <nvs_flash.h>

#include "gtest/gtest.h"

namespace {

class NvsEnvironment : public testing::Environment {
 public:
  void SetUp() override { ASSERT_EQ(ESP_OK, nvs_flash_init()); }
};

testing::Environment* const nvs_environment =
    testing::AddGlobalTestEnvironment(new NvsEnvironment());

}  // namespace

#endif  // ROO_PREFS_USE_ESP32_NVS
