exports_files(["LICENSE"])

cc_library(
    name = "hiai_header",
    hdrs = glob([
        "DDK/ai_ddk_mixmodel_lib/include/*.h",
    ]),
    includes = ["DDK/ai_ddk_mixmodel_lib/include/"],
)

cc_library(
    name = "hiai_arm64-v8a",
    srcs = glob([
        "DDK/ai_ddk_mixmodel_lib/lib64/libhiai.so",
    ]),
    deps = ["hiai_header"],
    visibility = ["//visibility:public"],
)
