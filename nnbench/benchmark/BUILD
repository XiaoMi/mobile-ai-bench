package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "//nnbench:nnbench.bzl",
    "if_android",
    "if_mace_enabled",
    "if_ncnn_enabled",
    "if_snpe_enabled",
)

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
    copts = if_mace_enabled([
        "-DNNBENCH_ENABLE_MACE",
    ]) + if_ncnn_enabled([
        "-DNNBENCH_ENABLE_NCNN",
    ]) + if_snpe_enabled([
        "-DNNBENCH_ENABLE_SNPE",
    ]),
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
    ] + if_mace_enabled([
        "//nnbench/executors:mace_executor",
    ]) + if_ncnn_enabled([
        "//nnbench/executors:ncnn_executor",
    ]) + if_snpe_enabled([
        "//nnbench/executors:snpe_executor",
    ]),
)
