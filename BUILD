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
        "//lib/roo_backport",
        "//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/Preferences",
    ]
)
