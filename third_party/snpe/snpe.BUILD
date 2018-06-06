exports_files(["LICENSE.txt"])

cc_library(
    name = "snpe_hdr",
    hdrs = glob([
        "include/zdl/*/*.hpp",
    ]),
    includes = ["include/zdl"]
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
        "lib/arm-android-gcc4.9/libSNPE.so",
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "snpe_arm64-v8a",
    srcs = [
        "lib/aarch64-android-gcc4.9/libSNPE.so"
        #"lib/aarch64-linux-gcc4.9/libSNPE.so"
    ],
    deps = ["snpe_hdr"],
    visibility = ["//visibility:public"],
)
