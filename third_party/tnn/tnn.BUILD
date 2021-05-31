exports_files(["LICENSE"])

cc_library(
    name = "tnn_hdr",
    hdrs = glob([
        "include/tnn/*.h",
        "include/tnn/*/*.h",
    ]),
    includes = ["include"],
)

cc_library(
    name = "tnn_armeabi-v7a",
    srcs = [
        "armeabi-v7a/libTNN.so",
    ],
    deps = ["tnn_hdr"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "tnn_arm64-v8a",
    srcs = [
        "arm64-v8a/libTNN.so",
    ],
    deps = ["tnn_hdr"],
    visibility = ["//visibility:public"],
)



