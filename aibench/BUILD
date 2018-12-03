config_setting(
    name = "android",
    values = {
        "crosstool_top": "//external:android/crosstool",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "android_armv7",
    values = {
        "crosstool_top": "//external:android/crosstool",
        "cpu": "armeabi-v7a",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "android_arm64",
    values = {
        "crosstool_top": "//external:android/crosstool",
        "cpu": "arm64-v8a",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "armhf_linux",
    values = {
        "crosstool_top": "//tools/arm_compiler:toolchain",
        "cpu": "armeabi-v7a"
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "aarch64_linux",
    values = {
        "crosstool_top": "//tools/aarch64_compiler:toolchain",
        "cpu": "aarch64"
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "mace_enabled",
    define_values = {
        "mace": "true",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "ncnn_enabled",
    define_values = {
        "ncnn": "true",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "snpe_enabled",
    define_values = {
        "snpe": "true",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "tflite_enabled",
    define_values = {
        "tflite": "true",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "opencv_enabled",
    define_values = {
        "opencv": "true",
    },
    visibility = ["//visibility:public"],
)
