
# -*- Python -*-



def getCMakeToolchain():
    return select({
        "@aibench//aibench:android_armv7": "$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake",
        "@aibench//aibench:android_arm64": "$$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake",
        "@aibench//aibench:aarch64_linux": "$(rootpath @aibench//tools/cmake_toolchain:cmakes)",
        "@aibench//aibench:armhf_linux": "$(rootpath @aibench//tools/cmake_toolchain:cmakes)",
        "//conditions:default": "",
    })

