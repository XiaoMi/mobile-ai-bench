exports_files(["LICENSE"])

cc_library(
    name = "opencv_header",
    hdrs = glob([
        "sdk/native/jni/include/**/*.hpp",
    ]),
    includes = ["sdk/native/jni/include"],
)

cc_library(
    name = "opencv_x86_64",
    srcs = glob([
        "sdk/native/libs/x86_64/libopencv_java3.so",
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
    name = "opencv_armeabi-v7a",
    srcs = glob([
        "sdk/native/libs/armeabi-v7a/libopencv_java3.so",
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
        "sdk/native/libs/arm64-v8a/libopencv_java3.so",
    ]),
    linkopts = [
        "-ljnigraphics",
        "-lz",
        "-ldl",
        "-lm",
        "-llog",
    ],
    deps = ["opencv_header"],
    visibility = ["//visibility:public"],
)
