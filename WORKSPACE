workspace(name = "aibench")

new_http_archive(
    name = "gtest",
    build_file = "third_party/googletest/googletest.BUILD",
    sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
    strip_prefix = "googletest-release-1.8.0",
    urls = [
        "https://cnbj1.fds.api.xiaomi.com/mace/third-party/googletest/googletest-release-1.8.0.zip",
        "https://github.com/google/googletest/archive/release-1.8.0.zip",
    ],
)

new_http_archive(
    name = "ncnn",
    build_file = "third_party/ncnn/ncnn.BUILD",
    sha256 = "de85593597a6b0a3c602c25c5c752d8c216f34da19042c7fd93f323916f5537b",
    strip_prefix = "ncnn-20180830",
    type = "zip",
    urls = [
        "https://cnbj1.fds.api.xiaomi.com/aibench/third_party/ncnn-20180830.zip",
        "https://codeload.github.com/Tencent/ncnn/zip/20180830",
    ],
)

# You need to comment following new_http_archive and uncomment following
# new_local_repository to benchmark SNPE
new_http_archive(
    name = "snpe",
    build_file = "third_party/snpe/snpe.BUILD",
    sha256 = "6f40cdeb86a1afd25b8bc7a41981b55b5f2db59f82e2aaf4c8951a9c5472ef4e",
    strip_prefix = "snpe-1.18.0",
    urls = [
        "https://cnbj1-fds.api.xiaomi.net/aibench/third_party/snpe-1.18.0_with_libgnustl_shared.so.zip",
    ],
)
# You need to uncomment following new_local_repository and comment foregoing
# new_http_archive to benchmark SNPE
# new_local_repository(
#     name = "snpe",
#     build_file = "third_party/snpe/snpe.BUILD",
#     path = "/path/to/snpe-1.18.0",
# )

http_archive(
    # v2.2.0 + fix of include path
    name = "com_github_gflags_gflags",
    sha256 = "16903f6bb63c00689eee3bf7fb4b8f242934f6c839ce3afc5690f71b712187f9",
    strip_prefix = "gflags-30dbc81fb5ffdc98ea9b14b1918bfe4e8779b26e",
    urls = [
        "https://cnbj1.fds.api.xiaomi.com/mace/third-party/gflags/gflags-30dbc81fb5ffdc98ea9b14b1918bfe4e8779b26e.zip",
        "https://github.com/gflags/gflags/archive/30dbc81fb5ffdc98ea9b14b1918bfe4e8779b26e.zip",
    ],
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

bind(
    name = "gflags_nothreads",
    actual = "@com_github_gflags_gflags//:gflags_nothreads",
)

# Set up Android NDK
android_ndk_repository(
    name = "androidndk",
    # Android 5.0
    api_level = 21,
)
