licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "tflite_headers",
    hdrs = glob([
        "tensorflow/lite/*.h",
        "tensorflow/lite/*/*.h",
        "tensorflow/lite/*/*/*.h",
        "flatbuffers/*.h",
        "absl/base/*.h",
    ]),
    visibility = ["//visibility:public"],
    strip_include_prefix = "",
)

cc_library(
    name = "tflite_armeabi-v7a",
    srcs = [
        "tensorflow/lite/lib/armeabi-v7a/libtensorflowlite.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["tflite_headers"],
)

cc_library(
    name = "tflite_arm64-v8a",
    srcs = [
        "tensorflow/lite/lib/arm64-v8a/libtensorflowlite.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["tflite_headers"],
)
