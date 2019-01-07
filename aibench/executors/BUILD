package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "//aibench:aibench.bzl",
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
    deps = [
        "//aibench/proto:aibench_proto_cc",
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
        "//third_party/mace:mace",
    ],
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
    ])+ if_android_arm64([
        "@snpe//:snpe_arm64-v8a",
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
    ],
    deps = [
        ":base_executor",
        "@ncnn",
    ],
)

cc_library(
    name = "tflite_executor",
    srcs = glob([
        "tflite/tflite_executor.cc",
    ]),
    hdrs = [
        "tflite/tflite_executor.h",
    ],
    deps = [
        ":base_executor",
    ] + if_android_armv7([
        "//third_party/tflite:tflite_armeabi-v7a",
    ]) + if_android_arm64([
        "//third_party/tflite:tflite_arm64-v8a",
    ]),
)
