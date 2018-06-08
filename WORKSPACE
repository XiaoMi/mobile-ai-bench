workspace(name = "nnbench")

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
    sha256 = "a813f42ed898c008d18e7abf0f0e8fb0b6c5d8de1e7a9e9c8a2d0b3967678193",
    strip_prefix = "ncnn-20180427",
    type = "zip",
    urls = [
        "https://cnbj1-fds.api.xiaomi.net/nnbench/third_party/ncnn-20180427.zip",
        "https://codeload.github.com/Tencent/ncnn/zip/20180427",
    ],
)

new_http_archive(
    name = "snpe",
    build_file = "third_party/snpe/snpe.BUILD",
    sha256 = "b11780e5e7f591e916c69bdface4a1ef75b0c19f7b43c868bd62c0f3747d3fbb",
    strip_prefix = "snpe-1.15.0",
    urls = [
        "https://cnbj1-fds.api.xiaomi.net/nnbench/third_party/snpe-1.15.0.zip",
    ],
)

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
