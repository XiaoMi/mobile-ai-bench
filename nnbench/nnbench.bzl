# -*- Python -*-

def if_android(a):
    return select({
        "//nnbench:android": a,
        "//conditions:default": [],
    })

def if_not_android(a):
    return select({
        "//nnbench:android": [],
        "//conditions:default": a,
    })

def if_android_armv7(a):
    return select({
        "//nnbench:android_armv7": a,
        "//conditions:default": [],
    })

def if_android_arm64(a):
    return select({
        "//nnbench:android_arm64": a,
        "//conditions:default": [],
    })

def if_mace_enabled(a):
    return select({
        "//nnbench:mace_enabled": a,
        "//conditions:default": [],
    })

def if_ncnn_enabled(a):
    return select({
        "//nnbench:ncnn_enabled": a,
        "//conditions:default": [],
    })

def if_snpe_enabled(a):
    return select({
        "//nnbench:snpe_enabled": a,
        "//conditions:default": [],
    })
