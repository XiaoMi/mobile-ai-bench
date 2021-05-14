exports_files(["LICENSE.txt"])

cc_library(
    name = "snpe_hdr",
    hdrs = glob([
        "include/zdl/*/*.hpp",
    ]),
    includes = ["include/zdl"],
)

cc_library(
    name = "snpe_x86_64",
    srcs = [
        "lib/x86_64-linux-clang/libSNPE.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_armeabi-v7a",
    srcs = [
        "lib/arm-android-clang6.0/libSNPE.so",
        "lib/arm-android-clang6.0/libc++_shared.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_arm64-v8a",
    linkopts = [
        "-llog",
    ],
    srcs = [
        "lib/aarch64-android-clang6.0/libSNPE.so",
        "lib/aarch64-android-clang6.0/libc++_shared.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_armhf",
    srcs = [
        "lib/arm-oe-linux-gcc8.2hf/libSNPE.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_aarch64",
    srcs = [
        "lib/aarch64-linux-gcc4.9/libSNPE.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)



