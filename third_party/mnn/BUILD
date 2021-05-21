licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "mnn_headers",
    hdrs = glob([
        "include/MNN/*.h",
        "include/MNN/*.hpp",
        "tools/*",
        "tools/*/*.hpp",
        "tools/*/*/*.h",
        "tools/*/*/*/*/*.h",
        "schema/*/*.h",
        "3rd_party/*/*/*/*.h",
    ]),
    includes = ["include"],
    strip_include_prefix = "",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "mnn_armeabi-v7a",
    srcs = [
        "project/android/build_32/libMNN.so",
        "project/android/build_32/libMNN_Express.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mnn_headers"],
)

cc_library(
    name = "mnn_arm64-v8a",
    srcs = [
        "project/android/build_64/libMNN.so",
        "project/android/build_64/libMNN_Express.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mnn_headers"],
)
