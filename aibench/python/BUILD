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
