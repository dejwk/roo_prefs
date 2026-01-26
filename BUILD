load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "roo_prefs",
    srcs = glob(
        [
            "src/**/*.h",
            "src/**/*.cpp",
        ],
        exclude = ["test/**"],
    ),
    includes = [
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@roo_backport",
        "@roo_logging",
        "@roo_scheduler",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/Preferences",
    ],
)

cc_test(
    name = "prefs_test",
    size = "small",
    srcs = [
        "test/prefs_test.cpp",
    ],
    copts = ["-Iexternal/gtest/include"],
    includes = ["src"],
    linkstatic = 1,
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
    ],
    copts = ["-Iexternal/gtest/include"],
    includes = ["src"],
    linkstatic = 1,
    deps = [
        ":roo_prefs",
        "@roo_testing//:arduino_gtest_main",
    ],
)
