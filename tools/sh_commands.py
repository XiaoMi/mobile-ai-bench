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
import hashlib
import os
import re
import sh
import urllib


FRAMEWORKS = (
    "MACE",
    "SNPE",
    "NCNN",
    "TFLITE"
)


RUNTIMES = (
    "CPU",
    "GPU",
    "DSP"
)


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


def file_checksum(fname):
    hash_func = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_func.update(chunk)
    return hash_func.hexdigest()


def adb_push_file(src_file, dst_dir, serialno):
    src_checksum = file_checksum(src_file)
    dst_file = os.path.join(dst_dir, os.path.basename(src_file))
    stdout_buff = []
    try:
        sh.adb("-s", serialno, "shell", "md5sum", dst_file,
               _out=lambda line: stdout_buff.append(line))
    except sh.ErrorReturnCode_1:
        print("Push %s to %s" % (src_file, dst_dir))
        sh.adb("-s", serialno, "push", src_file, dst_dir)
    else:
        dst_checksum = stdout_buff[0].split()[0]
        if src_checksum == dst_checksum:
            print("Equal checksum with %s and %s" % (src_file, dst_file))
        else:
            print("Push %s to %s" % (src_file, dst_dir))
            sh.adb("-s", serialno, "push", src_file, dst_dir)


def adb_push(src_path, dst_dir, serialno):
    if os.path.isdir(src_path):
        for src_file in os.listdir(src_path):
            adb_push_file(os.path.join(src_path, src_file), dst_dir, serialno)
    else:
        adb_push_file(src_path, dst_dir, serialno)


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


def download_file(configs, file_name, output_dir):
    file_path = output_dir + "/" + file_name
    url = configs[file_name]
    checksum = configs[file_name + "_md5_checksum"]
    if not os.path.exists(file_path) or file_checksum(file_path) != checksum:
        print("downloading %s..." % file_name)
        urllib.urlretrieve(url, file_path)
    if file_checksum(file_path) != checksum:
        print("file %s md5 checksum not match" % file_name)
        exit(1)
    return file_path


def get_mace(configs, abis, output_dir, build_mace):
    if build_mace:
        sh.bash("tools/build_mace.sh", abis, os.path.abspath(output_dir),
                _fg=True)
    else:
        file_path = download_file(configs, "libmace-a372fa5e.zip", output_dir)
        sh.unzip("-o", file_path, "-d", "third_party/mace")
        pass


def get_tflite(configs, output_dir):
    file_path = download_file(configs, "tensorflow-1.10.1.zip", output_dir)
    sh.unzip("-o", file_path, "-d", "third_party/tflite")


def bazel_build(serialno, target, abi, frameworks, runtimes):
    print("* Build %s with ABI %s" % (target, abi))
    if abi == "host":
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
            "--cpu=%s" % abi,
            "--action_env=ANDROID_NDK_HOME=%s"
            % os.environ["ANDROID_NDK_HOME"],
        )
    for framework in frameworks:
        bazel_args += ("--define", "%s=true" % framework.lower())
    if "DSP" in runtimes and abi == "armeabi-v7a":
        with device_lock(serialno):
            try:
                output = sh.adb("-s", serialno, "shell",
                                "ls /system/lib/libcdsprpc.so")
            except sh.ErrorReturnCode_1:
                print("/system/lib/libcdsprpc.so does not exists! Skip DSP.")
            else:
                if "No such file or directory" in output:
                    print("/system/lib/libcdsprpc.so does not exists! Skip DSP.")  # noqa
                else:
                    bazel_args += ("--define", "dsp=true")
    sh.bazel(
        _fg=True,
        *bazel_args)
    print("Build done!\n")


def bazel_target_to_bin(target):
    # change //aibench/a/b:c to bazel-bin/aibench/a/b/c
    prefix, bin_name = target.split(':')
    prefix = prefix.replace('//', '/')
    if prefix.startswith('/'):
        prefix = prefix[1:]
    host_bin_path = "bazel-bin/%s" % prefix
    return host_bin_path, bin_name


def prepare_device_env(serialno, abi, device_bin_path, frameworks):
    # for snpe
    if "SNPE" in frameworks:
        snpe_lib_path = ""
        if abi == "armeabi-v7a":
            snpe_lib_path = \
                "bazel-mobile-ai-bench/external/snpe/lib/arm-android-gcc4.9"
        elif abi == "arm64-v8a":
            snpe_lib_path = \
                "bazel-mobile-ai-bench/external/snpe/lib/aarch64-android-gcc4.9"  # noqa

        if snpe_lib_path:
            adb_push(snpe_lib_path, device_bin_path, serialno)
            libgnustl_path = os.environ["ANDROID_NDK_HOME"] + \
                "/sources/cxx-stl/gnu-libstdc++/4.9/libs/%s/" \
                "libgnustl_shared.so" % abi
            adb_push(libgnustl_path, device_bin_path, serialno)

        adb_push("bazel-mobile-ai-bench/external/snpe/lib/dsp",
                 device_bin_path, serialno)

    # for mace
    if "MACE" in frameworks and abi == "armeabi-v7a":
        adb_push("third_party/mace/lib/armeabi-v7a/cpu_gpu_dsp/libhexagon_controller.so",  # noqa
                 device_bin_path, serialno)

    # for tflite
    if "TFLITE" in frameworks:
        tflite_lib_path = ""
        if abi == "armeabi-v7a":
            tflite_lib_path = \
               "third_party/tflite/tensorflow/contrib/lite/" + \
               "lib/armeabi-v7a/libtensorflowLite.so"
        elif abi == "arm64-v8a":
            tflite_lib_path = \
               "third_party/tflite/tensorflow/contrib/lite/" + \
               "lib/arm64-v8a/libtensorflowLite.so"
        if tflite_lib_path:
            adb_push(tflite_lib_path, device_bin_path, serialno)


def prepare_model_and_input(serialno, models_inputs, device_bin_path,
                            output_dir):
    file_names = [f for f in models_inputs if not f.endswith("_md5_checksum")]
    for file_name in file_names:
        file_path = models_inputs[file_name]
        local_file_path = file_path
        if file_path.startswith("http"):
            local_file_path = \
                download_file(models_inputs, file_name, output_dir)
        else:
            checksum = models_inputs[file_name + "_md5_checksum"]
            if file_checksum(local_file_path) != checksum:
                print("file %s md5 checksum not match" % file_name)
                exit(1)
        adb_push(local_file_path, device_bin_path, serialno)


def prepare_all_model_and_input(serialno, configs, device_bin_path, output_dir,
                                frameworks, build_mace):
    models_inputs = configs["models_and_inputs"]
    if "MACE" in frameworks:
        if build_mace:
            # mace model files are generated from source
            for model_file in os.listdir(output_dir):
                if model_file.endswith(".pb") or model_file.endswith(".data"):
                    model_file_path = output_dir + '/' + model_file
                    adb_push(model_file_path, device_bin_path, serialno)
        else:
            prepare_model_and_input(serialno, models_inputs["MACE"],
                                    device_bin_path, output_dir)
    if "SNPE" in frameworks:
        prepare_model_and_input(serialno, models_inputs["SNPE"],
                                device_bin_path, output_dir)
    if "TFLITE" in frameworks:
        prepare_model_and_input(serialno, models_inputs["TFLITE"],
                                device_bin_path, output_dir)

    # ncnn model files are generated from source
    if "NCNN" in frameworks:
        ncnn_model_path = "bazel-genfiles/external/ncnn/models/"
        adb_push(ncnn_model_path, device_bin_path, serialno)
        prepare_model_and_input(serialno, models_inputs["NCNN"],
                                device_bin_path, output_dir)


def get_cpu_mask(serialno):
    freq_list = []
    cpu_id = 0
    cpu_mask = ''
    while (True):
        try:
            freq_list.append(
                int(sh.adb("-s", serialno, "shell",
                           "cat /sys/devices/system/cpu/cpu%d"
                           "/cpufreq/cpuinfo_max_freq" % cpu_id)))
        except (ValueError, sh.ErrorReturnCode_1):
            break
        else:
            cpu_id += 1
    for freq in freq_list:
        cpu_mask = '1' + cpu_mask if freq == max(freq_list) else '0' + cpu_mask
    return(str(hex(int(cpu_mask, 2)))[2:])


def adb_run(abi,
            serialno,
            configs,
            host_bin_path,
            bin_name,
            run_interval,
            num_threads,
            build_mace,
            frameworks=None,
            model_names=None,
            runtimes=None,
            device_bin_path="/data/local/tmp/aibench",
            output_dir="output",
            ):
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
        try:
            sh.bash("tools/power.sh",
                    serialno, props["ro.board.platform"],
                    _fg=True)
        except Exception, e:
            print("Config power exception %s" % str(e))

        sh.adb("-s", serialno, "shell", "mkdir -p %s" % device_bin_path)
        sh.adb("-s", serialno, "shell", "rm -rf %s"
               % os.path.join(device_bin_path, "interior"))
        sh.adb("-s", serialno, "shell", "mkdir %s"
               % os.path.join(device_bin_path, "interior"))
        prepare_device_env(serialno, abi, device_bin_path, frameworks)
        prepare_all_model_and_input(serialno, configs, device_bin_path,
                                    output_dir, frameworks, build_mace)
        adb_push(host_bin_full_path, device_bin_path, serialno)

        print("Run %s" % device_bin_full_path)

        stdout_buff = []
        process_output = make_output_processor(stdout_buff)
        cpu_mask = get_cpu_mask(serialno)
        cmd = "cd %s; ADSP_LIBRARY_PATH='.;/system/lib/rfsa/adsp;/system" \
              "/vendor/lib/rfsa/adsp;/dsp'; LD_LIBRARY_PATH=. " \
              "taskset " % device_bin_path + cpu_mask + " ./model_benchmark"

        for runtime in runtimes:
            for framework in frameworks:
                for model_name in model_names:
                    print(framework, runtime, model_name)
                    args = "--run_interval=%d --num_threads=%d " \
                           "--framework=%s --runtime=%s --model_name=%s " \
                           "--product_soc=%s.%s" % \
                           (run_interval, num_threads, framework, runtime,
                            model_name,
                            props["ro.product.model"].replace(" ", ""),
                            props["ro.board.platform"])
                    sh.adb(
                        "-s",
                        serialno,
                        "shell",
                        "%s %s" % (cmd, args),
                        _tty_in=True,
                        _out=process_output,
                        _err_to_out=True)
        return "".join(stdout_buff)
