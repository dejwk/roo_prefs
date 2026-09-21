load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "roo_prefs",
    srcs = glob(
        [
            "src/**/*.h",
            "src/**/*.cpp",
        ],
        exclude = [
            "src/roo_prefs/lazy_write_pref.h",
            "src/roo_prefs/store/filesystem_store.h",
            "src/roo_prefs/store/filesystem_store.ipp",
            "test/**",
        ],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_backport",
        "@roo_logging",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_esp_idf": [
            "@roo_testing//roo_testing/frameworks/esp-idf:headers",
            "@roo_testing//roo_testing/frameworks/esp32_shims:idf",
        ],
        "//conditions:default": [
            "@roo_testing//roo_testing/frameworks/arduino-esp32/libraries/Preferences",
        ],
    }),
)

# Optional lazy-write API. Users must also depend directly on roo_scheduler.
cc_library(
    name = "lazy_write_pref",
    hdrs = ["src/roo_prefs/lazy_write_pref.h"],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [":roo_prefs"],
)

cc_library(
    name = "filesystem_store",
    hdrs = [
        "src/roo_prefs/store/filesystem_store.h",
        "src/roo_prefs/store/filesystem_store.ipp",
    ],
    includes = ["src"],
    visibility = ["//visibility:public"],
    deps = [
        ":roo_prefs",
        "@roo_io",
    ],
)

cc_test(
    name = "filesystem_store_test",
    size = "small",
    srcs = ["test/filesystem_store_test.cpp"],
    deps = [
        ":filesystem_store",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_idf": ["@roo_testing//:esp_idf_gtest_main"],
        "//conditions:default": ["@roo_testing//:arduino_gtest_main"],
    }),
)

cc_test(
    name = "prefs_test",
    size = "small",
    srcs = [
        "test/nvs_test_environment.cpp",
        "test/prefs_test.cpp",
    ],
    includes = ["src"],
    linkstatic = 1,
    deps = [
        ":roo_prefs",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_idf": ["@roo_testing//:esp_idf_gtest_main"],
        "//conditions:default": ["@roo_testing//:arduino_gtest_main"],
    }),
)

cc_test(
    name = "arduino_string_pref_test",
    size = "small",
    srcs = [
        "test/arduino_string_pref_test.cpp",
    ],
    includes = ["src"],
    linkstatic = 1,
    target_compatible_with = [
        "@roo_testing//roo_testing/platforms:arduino",
    ],
    deps = [
        ":roo_prefs",
        "@roo_testing//:arduino_gtest_main",
    ],
)

cc_test(
    name = "lazy_write_pref_test",
    size = "small",
    srcs = [
        "test/lazy_write_pref_test.cpp",
        "test/nvs_test_environment.cpp",
    ],
    includes = ["src"],
    linkstatic = 1,
    deps = [
        ":lazy_write_pref",
        "@roo_scheduler",
        "@roo_testing//roo_testing/system:manual_time_mode",
    ] + select({
        "@roo_testing//roo_testing/platforms:is_idf": ["@roo_testing//:esp_idf_gtest_main"],
        "//conditions:default": ["@roo_testing//:arduino_gtest_main"],
    }),
)
