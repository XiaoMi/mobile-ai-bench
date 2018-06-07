# Copyright 2018 Xiaomi, Inc.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import filelock
import os
import re
import sh
import urllib
import yaml


def strip_invalid_utf8(str):
    return sh.iconv(str, "-c", "-t", "UTF-8")


def split_stdout(stdout_str):
    stdout_str = strip_invalid_utf8(stdout_str)
    # Filter out last empty line
    return [l.strip() for l in stdout_str.split('\n') if len(l.strip()) > 0]


def make_output_processor(buff):
    def process_output(line):
        print(line.rstrip())
        buff.append(line)

    return process_output


def device_lock_path(serialno):
    return "/tmp/device-lock-%s" % serialno


def device_lock(serialno, timeout=3600):
    return filelock.FileLock(device_lock_path(serialno), timeout=timeout)


def adb_devices():
    serialnos = []
    p = re.compile(r'(\w+)\s+device')
    for line in split_stdout(sh.adb("devices")):
        m = p.match(line)
        if m:
            serialnos.append(m.group(1))

    return serialnos


def adb_getprop_by_serialno(serialno):
    outputs = sh.adb("-s", serialno, "shell", "getprop")
    raw_props = split_stdout(outputs)
    props = {}
    p = re.compile(r'\[(.+)\]: \[(.+)\]')
    for raw_prop in raw_props:
        m = p.match(raw_prop)
        if m:
            props[m.group(1)] = m.group(2)
    return props


def adb_supported_abis(serialno):
    props = adb_getprop_by_serialno(serialno)
    abilist_str = props["ro.product.cpu.abilist"]
    abis = [abi.strip() for abi in abilist_str.split(',')]
    return abis


def adb_push(src_path, dst_path, serialno):
    print("Push %s to %s" % (src_path, dst_path))
    sh.adb("-s", serialno, "push", src_path, dst_path)


def get_soc_serialnos_map():
    serialnos = adb_devices()
    soc_serialnos_map = {}
    for serialno in serialnos:
        props = adb_getprop_by_serialno(serialno)
        soc_serialnos_map.setdefault(props["ro.board.platform"], []) \
            .append(serialno)

    return soc_serialnos_map


def get_target_socs_serialnos(target_socs=None):
    soc_serialnos_map = get_soc_serialnos_map()
    serialnos = []
    if target_socs is None:
        target_socs = soc_serialnos_map.keys()
    for target_soc in target_socs:
        serialnos.extend(soc_serialnos_map[target_soc])
    return serialnos


def bazel_build(target,
                abi="armeabi-v7a"):
    print("* Build %s with ABI %s" % (target, abi))
    if abi == "x86_64":
        bazel_args = (
            "build",
            target,
        )
    else:
        bazel_args = (
            "build",
            target,
            "--config",
            "android",
            "--cpu=%s" % abi,)
    sh.bazel(
        _fg=True,
        *bazel_args)
    print("Build done!\n")


def bazel_target_to_bin(target):
    # change //nnbench/a/b:c to bazel-bin/nnbench/a/b/c
    prefix, bin_name = target.split(':')
    prefix = prefix.replace('//', '/')
    if prefix.startswith('/'):
        prefix = prefix[1:]
    host_bin_path = "bazel-bin/%s" % prefix
    return host_bin_path, bin_name


def prepare_device_env(serialno, abi="armeabi-v7a",
                       device_bin_path="/data/local/tmp/nnbench"):
    sh.adb("-s", serialno, "shell", "mkdir -p %s" % device_bin_path)
    # for snpe
    snpe_lib_path = ""
    if abi == "armeabi-v7a":
        snpe_lib_path = \
            "bazel-mobile-nn-bench/external/snpe/lib/arm-android-gcc4.9"
    elif abi == "arm64-v8a":
        snpe_lib_path = \
            "bazel-mobile-nn-bench/external/snpe/lib/aarch64-android-gcc4.9"

    if snpe_lib_path:
        adb_push(snpe_lib_path, device_bin_path, serialno)
        #sh.adb("-s", serialno, "push", snpe_lib_path, device_bin_path)
        libgnustl_path = os.environ["ANDROID_NDK_HOME"] + \
            "/sources/cxx-stl/gnu-libstdc++/4.9/libs/%s/libgnustl_shared.so" % \
            abi
        adb_push(libgnustl_path, device_bin_path, serialno)


def prepare_model_and_input(serialno, config_file, device_bin_path, output_dir):
    with open(config_file) as f:
        configs = yaml.load(f)

    for model_file in configs["models"]:
        host_model_path = output_dir + '/' + model_file
        device_model_path = device_bin_path + '/' + model_file
        urllib.urlretrieve(configs["models"][model_file], host_model_path)
        adb_push(host_model_path, device_model_path, serialno)

    for input_file in configs["inputs"]:
        host_input_path = output_dir + '/' + input_file
        urllib.urlretrieve(configs["inputs"][input_file], host_input_path)
        device_input_path = device_bin_path + '/' + input_file
        adb_push(host_input_path, device_input_path, serialno)


def adb_run(abi,
            serialno,
            host_bin_path,
            bin_name,
            args="",
            device_bin_path="/data/local/tmp/nnbench",
            model_and_input_config="tools/model_and_input.yml",
            output_dir="output"):
    host_bin_full_path = "%s/%s" % (host_bin_path, bin_name)
    device_bin_full_path = "%s/%s" % (device_bin_path, bin_name)
    props = adb_getprop_by_serialno(serialno)
    print(
        "====================================================================="
    )
    print("Trying to lock device %s" % serialno)
    with device_lock(serialno):
        print("Run on device: %s, %s, %s" %
              (serialno, props["ro.board.platform"],
               props["ro.product.model"]))
        sh.adb("-s", serialno, "shell", "rm -rf %s" % device_bin_path)
        prepare_device_env(serialno, abi, device_bin_path)
        prepare_model_and_input(serialno, model_and_input_config,
                                device_bin_path, output_dir)
        adb_push(host_bin_full_path, device_bin_full_path, serialno)

        print("Run %s" % device_bin_full_path)

        stdout_buff = []
        process_output = make_output_processor(stdout_buff)
        cmd = "cd %s; ADSP_LIBRARY_PATH='.;/system/lib/rfsa/adsp;/system" \
              "/vendor/lib/rfsa/adsp;/dsp'; LD_LIBRARY_PATH=. " \
              "./model_benchmark" % device_bin_path
        sh.adb(
            "-s",
            serialno,
            "shell",
            cmd,
            args,
            _tty_in=True,
            _out=process_output,
            _err_to_out=True)
        return "".join(stdout_buff)
