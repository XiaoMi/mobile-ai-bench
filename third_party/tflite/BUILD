licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "tflite_headers",
    hdrs = glob([
        "tensorflow/contrib/lite/*.h",
        "tensorflow/contrib/lite/*/*.h",
        "flatbuffers/*.h",
    ]),
    visibility = ["//visibility:public"],
    strip_include_prefix = "",
)

cc_library(
    name = "tflite_armeabi-v7a",
    srcs = [
        "tensorflow/contrib/lite/lib/armeabi-v7a/libtensorflowLite.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["tflite_headers"],
)

cc_library(
    name = "tflite_arm64-v8a",
    srcs = [
        "tensorflow/contrib/lite/lib/arm64-v8a/libtensorflowLite.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["tflite_headers"],
)

