licenses(["notice"])

exports_files(["LICENSE.txt"])

NCNN_HEADERS = [
    "include/allocator.h",
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

TPL_TOOLCHAIN_CMAKE = "toolchain.cmake.tpl"
AARCH64_TOOLCHAIN_CMAKE = "aarch64.toolchain.cmake"
ARMHF_TOOLCHAIN_CMAKE = "armhf.toolchain.cmake"

NCNN_CMAKE_OPTS = select({
    "@aibench//aibench:android_armv7": " -DCMAKE_TOOLCHAIN_FILE=$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" +
                                       " -DANDROID_ABI='armeabi-v7a'" +
                                       " -DANDROID_ARM_NEON=ON" +
                                       " -DANDROID_PLATFORM=android-14",
    "@aibench//aibench:android_arm64": " -DCMAKE_TOOLCHAIN_FILE=$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" +
                                       " -DANDROID_ABI='arm64-v8a'" +
                                       " -DANDROID_PLATFORM=android-21",
    "@aibench//aibench:aarch64_linux": " -DCMAKE_TOOLCHAIN_FILE=../" + AARCH64_TOOLCHAIN_CMAKE +
                                       " -DGENERIC_ABI='aarch64'" +
                                       " -DGENERIC_ARM_NEON=ON",
    "@aibench//aibench:armhf_linux":   " -DCMAKE_TOOLCHAIN_FILE=../" + ARMHF_TOOLCHAIN_CMAKE +
                                       " -DGENERIC_ABI='armhf'" +
                                       " -DGENERIC_ARM_NEON=ON",
    "//conditions:default": "",
})

genrule(
    name = "ncnn_gen_cmake_aarch64_toolchain",
    srcs = ["@aibench//tools/cmake_toolchain:toolchain", "@gcc_linaro_7_3_1_aarch64_linux_gnu//:gcc"],
    outs = [AARCH64_TOOLCHAIN_CMAKE],
    cmd = "workdir=$$(mktemp -d -t cmake_aarch64_toolchain-build.XXXXXXXXXX);" +
          "cp -aL $$(dirname $(location @aibench//tools/cmake_toolchain:toolchain)) $$workdir/;" +
          "mv $$workdir/cmake_toolchain/" + TPL_TOOLCHAIN_CMAKE + " $$workdir/cmake_toolchain/" + AARCH64_TOOLCHAIN_CMAKE +";" +
          "GCC_PATH=$$(realpath $(rootpath @gcc_linaro_7_3_1_aarch64_linux_gnu//:gcc));" +
          "GNU_PATH=$$(dirname $$(dirname $$GCC_PATH));" +
          "sed -i \"s^GCC_PATH^$$GCC_PATH^g\" $$workdir/cmake_toolchain/" + AARCH64_TOOLCHAIN_CMAKE + ";" +
          "sed -i \"s^GNU_PATH^$$GNU_PATH^g\" $$workdir/cmake_toolchain/" + AARCH64_TOOLCHAIN_CMAKE + ";" +
          "cp -a $$workdir/cmake_toolchain/* $(@D);" +
          "rm -rf $$workdir",
)

genrule(
    name = "ncnn_gen_cmake_armhf_toolchain",
    srcs = ["@aibench//tools/cmake_toolchain:toolchain", "@gcc_linaro_7_3_1_arm_linux_gnueabihf//:gcc"],
    outs = [ARMHF_TOOLCHAIN_CMAKE],
    cmd = "workdir=$$(mktemp -d -t cmake_armhf_toolchain-build.XXXXXXXXXX);" +
          "cp -aL $$(dirname $(location @aibench//tools/cmake_toolchain:toolchain)) $$workdir/;" +
          "mv $$workdir/cmake_toolchain/" + TPL_TOOLCHAIN_CMAKE + " $$workdir/cmake_toolchain/" + ARMHF_TOOLCHAIN_CMAKE +";" +
          "GCC_PATH=$$(realpath $(rootpath @gcc_linaro_7_3_1_arm_linux_gnueabihf//:gcc));" +
          "GNU_PATH=$$(dirname $$(dirname $$GCC_PATH));" +
          "sed -i \"s^GCC_PATH^$$GCC_PATH^g\" $$workdir/cmake_toolchain/" + ARMHF_TOOLCHAIN_CMAKE + ";" +
          "sed -i \"s^GNU_PATH^$$GNU_PATH^g\" $$workdir/cmake_toolchain/" + ARMHF_TOOLCHAIN_CMAKE + ";" +
          "cp -a $$workdir/cmake_toolchain/* $(@D);" +
          "rm -rf $$workdir",
)

CMAKE_TOOLCHAIN_SRC = select({
    "@aibench//aibench:aarch64_linux": [":ncnn_gen_cmake_aarch64_toolchain"],
    "@aibench//aibench:armhf_linux":   [":ncnn_gen_cmake_armhf_toolchain"],
    "//conditions:default": [],
})

COPY_CMAKE_COMMAND = select({
    "@aibench//aibench:aarch64_linux": "cp -aL $$(dirname $(location :ncnn_gen_cmake_aarch64_toolchain))/" + AARCH64_TOOLCHAIN_CMAKE + " $$workdir;",
    "@aibench//aibench:armhf_linux":   "cp -aL $$(dirname $(location :ncnn_gen_cmake_armhf_toolchain))/" + ARMHF_TOOLCHAIN_CMAKE + " $$workdir;",
    "//conditions:default": "",
})

genrule(
    name = "ncnn_gen",
    srcs = glob(["**/*"]) + CMAKE_TOOLCHAIN_SRC,
    outs = NCNN_HEADERS + NCNN_LIBRARIES + NCNN_MODELS,
    cmd = "workdir=$$(mktemp -d -t ncnn-build.XXXXXXXXXX);" +
          COPY_CMAKE_COMMAND +
          "cp -aL $$(dirname $(location CMakeLists.txt))/* $$workdir;" +
          "pushd $$workdir;" +
          "sed -i -e 's/fno-rtti/frtti/g' CMakeLists.txt;" +
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
)

cc_library(
    name = "ncnn",
    hdrs = NCNN_HEADERS,
    srcs = NCNN_LIBRARIES,
    include_prefix = "ncnn",
    visibility = ["//visibility:public"],
)
