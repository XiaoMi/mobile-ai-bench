licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "mace_headers",
    hdrs = glob([
        "include/mace/public/*.h",
    ]),
    strip_include_prefix = "include",
)

cc_library(
    name = "mace_armeabi-v7a",
    srcs = glob([
        "lib/armeabi-v7a/cpu_gpu_dsp/libmace.a",
    ]),
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "mace_arm64-v8a",
    srcs = glob([
        "lib/arm64-v8a/cpu_gpu/libmace.a",
    ]),
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "libhexagon",
    srcs = ["lib/armeabi-v7a/cpu_gpu_dsp/libhexagon_controller.so"],
    visibility = ["//visibility:public"],
)
