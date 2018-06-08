licenses(["notice"])

exports_files(["LICENSE.txt"])

NCNN_HEADERS = [
    "include/benchmark.h",
    "include/blob.h",
    "include/cpu.h",
    "include/layer.h",
    "include/layer_type_enum.h",
    "include/layer_type.h",
    "include/mat.h",
    "include/modelbin.h",
    "include/net.h",
    "include/opencv.h",
    "include/paramdict.h",
    "include/platform.h",
]

NCNN_LIBRARIES = [
    "lib/libncnn.a",
]

NCNN_MODELS = [
    "models/alexnet.param",
    "models/googlenet.param",
    "models/mobilenet.param",
    "models/mobilenet_ssd.param",
    "models/mobilenet_v2.param",
    "models/resnet18.param",
    "models/shufflenet.param",
    "models/squeezenet.param",
    "models/squeezenet_ssd.param",
    "models/vgg16.param",
]

NCNN_CMAKE_OPTS = select({
    "@nnbench//nnbench:android_armv7": " -DCMAKE_TOOLCHAIN_FILE=$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" +
                                       " -DANDROID_ABI='armeabi-v7a'" +
                                       " -DANDROID_ARM_NEON=ON" +
                                       " -DANDROID_PLATFORM=android-14",
    "@nnbench//nnbench:android_arm64": " -DCMAKE_TOOLCHAIN_FILE=$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" +
                                       " -DANDROID_ABI='arm64-v8a'" +
                                       " -DANDROID_PLATFORM=android-21",
    "//conditions:default": "",
})

genrule(
    name = "ncnn_gen",
    srcs = glob(["**/*"]),
    outs = NCNN_HEADERS + NCNN_LIBRARIES + NCNN_MODELS,
    cmd = "workdir=$$(mktemp -d -t ncnn-build.XXXXXXXXXX);" +
          "cp -aL $$(dirname $(location CMakeLists.txt))/* $$workdir;" +
          "pushd $$workdir;" +
          "mkdir build;" +
          "pushd build;" +
          "cmake " + NCNN_CMAKE_OPTS + " ..;" +
          "make -j4;" +
          "make install;" +
          "mkdir install/models;" +
          "cp ../benchmark/*.param install/models;" +
          "popd;" +
          "popd;" +
          "cp -a $$workdir/build/install/* $(@D);" +
          "rm -rf $$workdir",
    output_to_bindir = 0,
)

cc_library(
    name = "ncnn",
    hdrs = NCNN_HEADERS,
    srcs = NCNN_LIBRARIES,
    include_prefix = "ncnn",
    visibility = ["//visibility:public"],
)
