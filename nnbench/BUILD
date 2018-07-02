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
    name = "mace_enabled",
    define_values = {
        "mace": "true",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "mace_dsp_enabled",
    define_values = {
        "mace": "true",
    },
    values = {
        "cpu": "armeabi-v7a",
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
    values = {
        "cpu": "armeabi-v7a",
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
