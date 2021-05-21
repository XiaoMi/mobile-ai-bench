package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "//aibench:aibench.bzl",
    "if_aarch64_linux",
    "if_android",
    "if_android_arm64",
    "if_android_armv7",
    "if_armhf_linux",
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
	"//include:public_headers",
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
    linkopts = if_android([
        "-llog",
    ]),
    deps = [
        ":base_executor",
    ] + if_android_armv7([
        "//third_party/mace:mace_armeabi-v7a",
    ]) + if_android_arm64([
        "//third_party/mace:mace_arm64-v8a",
    ]) + if_aarch64_linux([
        "//third_party/mace:mace_aarch64",
    ]) + if_armhf_linux([
        "//third_party/mace:mace_armhf",
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
    ] + if_android_armv7([
        "@snpe//:snpe_armeabi-v7a",
    ]) + if_android_arm64([
        "@snpe//:snpe_arm64-v8a",
    ]) + if_aarch64_linux([
        "@snpe//:snpe_aarch64",
    ]) + if_armhf_linux([
        "@snpe//:snpe_armhf",
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

cc_library(
    name = "hiai_executor",
    srcs = glob([
        "hiai/hiai_executor.cc",
    ]),
    hdrs = [
        "hiai/hiai_executor.h",
    ],
    linkopts = [
        "-lstdc++",
    ],
    deps = [
        ":base_executor",
	"//aibench/utils:utils",
    ] + if_android_arm64([
        "@hiai//:hiai_arm64-v8a",
    ]),
)

cc_library(
    name = "mnn_executor",
    srcs = glob([
        "mnn/mnn_executor.cc",
    ]),
    hdrs = [
        "mnn/mnn_executor.h",
    ],
    deps = [
        ":base_executor",
    ] + if_android_armv7([
        "//third_party/mnn:mnn_armeabi-v7a",
    ]) + if_android_arm64([
        "//third_party/mnn:mnn_arm64-v8a",
    ]),
)
