package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "//nnbench:nnbench.bzl",
    "if_android_arm64",
    "if_android_armv7",
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
    name = "mace_executor",
    srcs = [
        "mace/mace_executor.cc",
    ],
    hdrs = [
        "mace/mace_executor.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-llog",
    ],
    deps = [
        ":base_executor",
    ] + if_android_armv7([
        "//third_party/nnlib:libhexagon",
        "//third_party/mace:mace_armeabi-v7a",
    ]) + if_android_arm64([
        "//third_party/mace:mace_arm64-v8a",
    ]),
)

cc_library(
    name = "snpe_executor",
    srcs = [
        "snpe/snpe_executor.cc",
    ],
    hdrs = [
        "snpe/snpe_executor.h",
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
    ]),
)

cc_library(
    name = "ncnn_executor",
    srcs = glob([
        "ncnn/ncnn_executor.cc",
    ]),
    hdrs = [
        "ncnn/ncnn_executor.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
        "-fno-rtti",  # ncnn builds with -fno-rtti
    ],
    deps = [
        ":base_executor",
        "@ncnn",
    ],
)
