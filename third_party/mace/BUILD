package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "mace_headers",
    hdrs = glob([
        "build/include/mace/*/*.h",
    ]),
    includes = ["build/include"],
    strip_include_prefix = "",
    visibility = ["//visibility:public"],
)


cc_library(
    name = "mace_x86_64",
    srcs = [
        "build/lib/linux-x86-64/libmace.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "mace_armeabi-v7a",
    srcs = [
        "build/lib/armeabi-v7a/libc++_shared.so",
        "build/lib/armeabi-v7a/libmace.so",
        "build/lib/armeabi-v7a/libhexagon_controller.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "mace_arm64-v8a",
    srcs = [
        "build/lib/arm64-v8a/libc++_shared.so",
        "build/lib/arm64-v8a/libmace.so",
        "build/lib/arm64-v8a/libhexagon_controller.so",
        "build/lib/arm64-v8a/libcdsprpc.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "mace_armhf",
    srcs = [
        "build/lib/arm_linux_gnueabihf/libmace.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)

cc_library(
    name = "mace_aarch64",
    srcs = [
        "build/lib/aarch64_linux_gnu/libmace.so",
    ],
    visibility = ["//visibility:public"],
    deps = ["mace_headers"],
)
