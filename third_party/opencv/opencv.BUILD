exports_files(["LICENSE"])

cc_library(
    name = "opencv_header",
    hdrs = glob([
        "include/**/*.hpp",
    ]),
    includes = ["include"],
)

cc_library(
    name = "opencv_armeabi-v7a",
    srcs = glob([
        "libs/armeabi-v7a/libopencv_java4.so",
    ]),
    linkopts = [
        "-lz",
        "-ldl",
        "-lm",
        "-llog",
    ],
    deps = ["opencv_header"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "opencv_arm64-v8a",
    srcs = glob([
        "libs/arm64-v8a/libopencv_java4.so",
    ]),
    linkopts = [
        "-lz",
        "-ldl",
        "-lm",
        "-llog",
    ],
    deps = ["opencv_header"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "opencv_aarch64_linux",
    srcs = glob([
        "libs/aarch64_linux/*.so.4.0",
    ]),
    linkopts = [
        "-ldl",
        "-lm",
    ],
    deps = ["opencv_header"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "opencv_armhf_linux",
    srcs = glob([
        "libs/armhf_linux/*.so.4.0",
    ]),
    linkopts = [
        "-ldl",
        "-lm",
    ],
    deps = ["opencv_header"],
    visibility = ["//visibility:public"],
)
