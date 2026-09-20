#include <cstdint>
#include <cstdio>

#include "esp_err.h"
#include "nvs_flash.h"
#include "roo_prefs.h"

namespace {

roo_prefs::Collection settings("device");
roo_prefs::String device_name(settings, "name", "unconfigured");
roo_prefs::Uint32 boot_count(settings, "boots");

bool InitializeNvs() {
  esp_err_t result = nvs_flash_init();
  if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
      result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    result = nvs_flash_erase();
    if (result == ESP_OK) result = nvs_flash_init();
  }
  if (result != ESP_OK) {
    std::fprintf(stderr, "NVS initialization failed: %s\n",
                 esp_err_to_name(result));
    return false;
  }
  return true;
}

}  // namespace

extern "C" void app_main() {
  if (!InitializeNvs()) return;

  if (!device_name.isSet() && !device_name.set("workbench")) {
    std::fprintf(stderr, "Failed to store the device name\n");
    return;
  }

  const uint32_t next_boot_count = boot_count.get() + 1;
  if (!boot_count.set(next_boot_count)) {
    std::fprintf(stderr, "Failed to update the boot count\n");
    return;
  }

  std::printf("Device: %s\n", device_name.get().c_str());
  std::printf("Boot count: %u\n", static_cast<unsigned>(boot_count.get()));
  std::fflush(stdout);
}
