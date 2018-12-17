py_library(
    name = "utils",
    srcs = glob([
        "utils/*.py",
    ]),
    srcs_version = "PY2AND3",
)

py_library(
    name = "evaluators",
    srcs = glob(
        [
            "evaluators/*.py",
        ],
        exclude = [
            "evaluators/*_test.py",
        ],
    ),
    srcs_version = "PY2AND3",
    deps = [
        ":utils",
    ],
)

py_test(
    name = "evaluator_test",
    srcs = ["evaluators/evaluator_test.py"],
    srcs_version = "PY2AND3",
    deps = [
        ":evaluators",
        ":utils",
    ],
)

py_library(
    name = "benchmark_lib",
    srcs = [
        "sh_commands.py",
        "utils/common.py",
    ],
    srcs_version = "PY2AND3",
    deps = [
        "//aibench/proto:aibench_proto_py",
    ],
)

py_binary(
    name = "benchmark",
    srcs = [
        "benchmark.py",
    ],
    srcs_version = "PY2AND3",
    deps = [
        "benchmark_lib",
    ],
)
