cc_library(
    name = "roo_prefs",
    visibility = ["//visibility:public"],
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
    deps = [
        "@roo_backport",
        "@roo_logging",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/Preferences",
    ]
)

cc_test(
    name = "prefs_test",
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
    size = "small",
)
