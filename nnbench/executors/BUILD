package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "//nnbench:nnbench.bzl",
    "if_android_armv7",
    "if_android_arm64",
    "if_not_android",
)

cc_library(
    name = "base_executor",
    srcs = [
        "base_executor.cc",
    ],
    hdrs = [
        "base_executor.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
    ],
)

cc_library(
    name = "snpe_executor",
    srcs = [
        "snpe/snpe_executor.cc",
        "snpe/snpe_utils.cc",
    ],
    hdrs = [
        "snpe/snpe_executor.h",
        "snpe/snpe_utils.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-pie",
        "-lm",
    ],
    deps = [
        ":base_executor",
    ] + if_not_android([
        "@snpe//:snpe_x86_64",
    ]) + if_android_armv7([
        "@snpe//:snpe_armeabi-v7a",
    ]) + if_android_arm64([
        "@snpe//:snpe_arm64-v8a",
    ]),
)
