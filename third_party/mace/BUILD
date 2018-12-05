package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "mace",
    deps = [
        "@mace//mace/codegen:generated_version",
        "@mace//mace/core",
        "@mace//mace/ops",
        "@mace//mace/libmace",
        "@mace//mace/utils",
        "@mace//mace/proto:mace_cc",
        "@mace//mace/codegen:generated_opencl",
        "@com_google_protobuf//:protobuf_lite",
    ],
)
