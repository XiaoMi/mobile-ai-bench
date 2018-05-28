package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "base_executor",
    srcs = [
        "base_executor.cc",
    ],
    hdrs = [
        "base_executor.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
    ],
)

cc_library(
    name = "snpe_executor",
    srcs = [
        "snpe/snpe_executor.cc",
    ],
    hdrs = [
        "snpe/snpe_executor.h",
    ],
    copts = [
        "-Werror",
        "-Wextra",
        "-Wno-missing-field-initializers",
    ],
    deps = [
        ":base_executor",
    ],
)
