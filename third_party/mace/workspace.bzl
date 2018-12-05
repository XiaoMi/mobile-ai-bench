load("@mace//repository/git:git_configure.bzl", "git_version_repository")
load("@mace//repository/opencl-kernel:opencl_kernel_configure.bzl", "encrypt_opencl_kernel_repository")

def mace_workspace():
    git_version_repository(name = "local_version_config")
    encrypt_opencl_kernel_repository(name = "local_opencl_kernel_encrypt")

    native.http_archive(
        name = "gemmlowp",
        sha256 = "4e9cd60f7871ae9e06dcea5fec1a98ddf1006b32a85883480273e663f143f303",
        strip_prefix = "gemmlowp-master-66fb41a7cafd2034a50e0b32791359897d657f7a",
        urls = [
            "https://cnbj1.fds.api.xiaomi.com/mace/third-party/gemmlowp/gemmlowp-master-66fb41a7cafd2034a50e0b32791359897d657f7a.zip",
        ],
    )

    native.new_http_archive(
        name = "opencl_headers",
        build_file = "third_party/mace/opencl-headers/opencl-headers.BUILD",
        sha256 = "b2b813dd88a7c39eb396afc153070f8f262504a7f956505b2049e223cfc2229b",
        strip_prefix = "OpenCL-Headers-f039db6764d52388658ef15c30b2237bbda49803",
        urls = [
            "https://cnbj1.fds.api.xiaomi.com/mace/third-party/OpenCL-Headers/f039db6764d52388658ef15c30b2237bbda49803.zip",
            "https://github.com/KhronosGroup/OpenCL-Headers/archive/f039db6764d52388658ef15c30b2237bbda49803.zip",
        ],
    )

    native.new_http_archive(
        name = "opencl_clhpp",
        build_file = "third_party/mace/opencl-clhpp/opencl-clhpp.BUILD",
        sha256 = "dab6f1834ec6e3843438cc0f97d63817902aadd04566418c1fcc7fb78987d4e7",
        strip_prefix = "OpenCL-CLHPP-4c6f7d56271727e37fb19a9b47649dd175df2b12",
        urls = [
            "https://cnbj1.fds.api.xiaomi.com/mace/third-party/OpenCL-CLHPP/OpenCL-CLHPP-4c6f7d56271727e37fb19a9b47649dd175df2b12.zip",
            "https://github.com/KhronosGroup/OpenCL-CLHPP/archive/4c6f7d56271727e37fb19a9b47649dd175df2b12.zip",
        ],
    )

    native.new_http_archive(
        name = "half",
        build_file = "third_party/mace/half/half.BUILD",
        sha256 = "0f514a1e877932b21dc5edc26a148ddc700b6af2facfed4c030ca72f74d0219e",
        strip_prefix = "half-code-356-trunk",
        urls = [
            "https://cnbj1.fds.api.xiaomi.com/mace/third-party/half/half-code-356-trunk.zip",
            "https://sourceforge.net/code-snapshots/svn/h/ha/half/code/half-code-356-trunk.zip",
        ],
    )

    native.http_archive(
        name = "tflite",
        sha256 = "c886d46ad8c91fcafed2d910ad9e7bc5aeb29856c387bdf9b6b4903cc16e6e60",
        strip_prefix = "tensorflow-mace-ffc8cc7e8c9d1894753509e88b17e251bc6255e3",
        urls = [
            "https://cnbj1.fds.api.xiaomi.com/mace/third-party/tflite/tensorflow-mace-ffc8cc7e8c9d1894753509e88b17e251bc6255e3.zip",
        ],
    )
