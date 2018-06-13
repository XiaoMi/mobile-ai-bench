package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load("//nnbench:nnbench.bzl", "if_android")

cc_library(
    name = "benchmark",
    testonly = 1,
    srcs = [
        "benchmark.cc",
    ],
    hdrs = [
        "benchmark.h",
    ],
    deps = [
        "//nnbench/executors:base_executor",
    ],
    alwayslink = 1,
)

cc_test(
    name = "model_benchmark",
    testonly = 1,
    srcs = ["benchmark_main.cc"],
    linkopts = [
        "-fopenmp",
        "-ldl",
    ] + if_android([
        "-pie",
        "-lm",
    ]),
    linkstatic = 1,
    deps = [
        "benchmark",
        "//external:gflags_nothreads",
        "//nnbench/executors:mace_executor",
        "//nnbench/executors:ncnn_executor",
        "//nnbench/executors:snpe_executor",
    ],
)
