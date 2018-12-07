# Description:
# mace proto.
#

package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # Apache 2.0

load(
    "@com_google_protobuf//:protobuf.bzl",
    "py_proto_library",
    "cc_proto_library",
)

py_proto_library(
    name = "aibench_proto_py",
    srcs = glob(["*.proto"]),
    default_runtime = "@com_google_protobuf//:protobuf_python",
    protoc = "@com_google_protobuf//:protoc",
    srcs_version = "PY2AND3",
    deps = ["@com_google_protobuf//:protobuf_python"],
)

cc_proto_library(
    name = "aibench_proto_cc",
    srcs = glob(["*.proto"]),
    default_runtime = "@com_google_protobuf//:protobuf_lite",
    protoc = "@com_google_protobuf//:protoc",
)

py_binary(
    name = "test",
    srcs = ["test.py"],
    srcs_version = "PY2AND3",
    deps = [
        ":aibench_proto_py",
    ],
)
