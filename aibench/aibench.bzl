# -*- Python -*-

def if_android(a):
    return select({
        "//aibench:android": a,
        "//conditions:default": [],
    })

def if_not_android(a):
    return select({
        "//aibench:android": [],
        "//conditions:default": a,
    })

def if_android_armv7(a):
    return select({
        "//aibench:android_armv7": a,
        "//conditions:default": [],
    })


def if_android_arm64(a):
    return select({
        "//aibench:android_arm64": a,
        "//conditions:default": [],
    })

def if_linux(a, default_value = []):
    return select({
        "//aibench:linux": a,
        "//conditions:default": [],
    })

def if_linux_base(a, default_value = []):
    return select({
        "//aibench:linux_base": a,
        "//conditions:default": default_value,
    })

def if_armhf_linux(a):
    return select({
        "//aibench:armhf_linux": a,
        "//conditions:default": [],
    })

def if_aarch64_linux(a):
    return select({
        "//aibench:aarch64_linux": a,
        "//conditions:default": [],
    })

def if_mace_enabled(a):
    return select({
        "//aibench:mace_enabled": a,
        "//conditions:default": [],
    })

def if_ncnn_enabled(a):
    return select({
        "//aibench:ncnn_enabled": a,
        "//conditions:default": [],
    })

def if_snpe_enabled(a):
    return select({
        "//aibench:snpe_enabled": a,
        "//conditions:default": [],
    })

def if_tflite_enabled(a):
    return select({
        "//aibench:tflite_enabled": a,
        "//conditions:default": [],
    })

def if_opencv_enabled(a):
    return select({
        "//aibench:opencv_enabled": a,
        "//conditions:default": [],
    })

def if_hiai_enabled(a):
    return select({
        "//aibench:hiai_enabled": a,
        "//conditions:default": [],
    })
def if_mnn_enabled(a):
    return select({
        "//aibench:mnn_enabled": a,
        "//conditions:default": [],
    })
