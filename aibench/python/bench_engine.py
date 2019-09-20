# Copyright 2018 The MobileAIBench Authors. All Rights Reserved.
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

import copy
import filelock
import hashlib
import os
import re
import sh
import time
import urllib

from aibench.proto import aibench_pb2
from aibench.proto import base_pb2
from aibench.python.utils import bench_utils
from aibench.python.utils import sh_commands
from aibench.python.utils.common import ABI_TOOLCHAIN_CONFIG
from aibench.python.utils.common import aibench_check
from aibench.python.device.device_manager import DeviceManager
from google.protobuf import text_format


class AIBenchKeyword(object):
    device_type = 'device_type'
    executor = 'executor'
    model_name = 'model_name'
    quantize = 'quantize'


def make_output_processor(buff):
    def process_output(line):
        print(line.rstrip())
        buff.append(line)

    return process_output


def device_lock_path(serialno):
    return "/tmp/device-lock-%s" % serialno


def device_lock(serialno, timeout=3600):
    return filelock.FileLock(device_lock_path(serialno), timeout=timeout)


def get_target_devices_by_socs(target_socs=None):
    devices = DeviceManager().list_devices()
    if target_socs is None:
        return devices
    valid_devices = []
    for device in devices:
        if device.target_soc in target_socs:
            valid_devices.extend(device)
    return valid_devices


def download_file(configs, filename, output_dir):
    file_path = output_dir + "/" + filename
    url = configs[filename]
    checksum = configs[filename + "_md5_checksum"]
    if not os.path.exists(file_path) or \
            bench_utils.file_checksum(file_path) != checksum:
        print("downloading %s..." % filename)
        urllib.urlretrieve(url, file_path)
    if bench_utils.file_checksum(file_path) != checksum:
        print("file %s md5 checksum not match" % filename)
        exit(1)
    return file_path


def get_tflite(configs, output_dir):
    file_path = download_file(configs, "tensorflow-1.10.1.zip", output_dir)
    sh.unzip("-o", file_path, "-d", "third_party/tflite")


def get_mnn(configs, output_dir):
    file_path = download_file(configs, "mnn-0.2.0.9.zip", output_dir)
    sh.unzip("-o", file_path, "-d", "third_party/mnn")


def bazel_build(target, abi, executor, device_types):
    print("* Build %s for %s with ABI %s"
          % (target, base_pb2.ExecutorType.Name(executor), abi))
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
            ABI_TOOLCHAIN_CONFIG[abi],
            "--cpu=%s" % abi,
            "--action_env=ANDROID_NDK_HOME=%s"
            % os.environ["ANDROID_NDK_HOME"],
        )
    bazel_args += ("--define", "%s=true"
                   % base_pb2.ExecutorType.Name(executor).lower())
    if executor == base_pb2.MACE:
        bazel_args += ("--define", "neon=true")
        bazel_args += ("--define", "openmp=true")
        bazel_args += ("--define", "opencl=true")
        bazel_args += ("--define", "quantize=true")
        if base_pb2.DSP in device_types:
            bazel_args += ("--define", "hexagon=true")

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


# TODO(luxuhui@xiaomi.com): opt the if-else.
def prepare_device_env(device, abi, device_bin_path, executor):
    opencv_lib_path = ""
    if abi == "armeabi-v7a":
        opencv_lib_path = 'bazel-mobile-ai-bench/external/opencv/libs/' + \
                           'armeabi-v7a/libopencv_java4.so'
    elif abi == "arm64-v8a":
        opencv_lib_path = 'bazel-mobile-ai-bench/external/opencv/libs/' + \
                           'arm64-v8a/libopencv_java4.so'
    elif abi == "armhf":
        opencv_lib_path = 'bazel-mobile-ai-bench/external/opencv/libs/' + \
                          'armhf_linux'
    elif abi == "aarch64":
        opencv_lib_path = 'bazel-mobile-ai-bench/external/opencv/libs/' + \
                          'aarch64_linux'
    if opencv_lib_path:
        device.push(opencv_lib_path, device_bin_path)
    # for snpe
    if base_pb2.SNPE == executor:
        snpe_lib_path = ""
        if abi == "armeabi-v7a":
            snpe_lib_path = \
                "bazel-mobile-ai-bench/external/snpe/lib/arm-android-gcc4.9"
        elif abi == "arm64-v8a":
            snpe_lib_path = \
                "bazel-mobile-ai-bench/external/snpe/lib/aarch64-android-gcc4.9"  # noqa

        if snpe_lib_path:
            device.push(snpe_lib_path, device_bin_path)
            libgnustl_path = os.environ["ANDROID_NDK_HOME"] + \
                ("/sources/cxx-stl/gnu-libstdc++/4.9/libs/%s/" % abi) + \
                "libgnustl_shared.so"
            device.push(libgnustl_path, device_bin_path)

        device.push("bazel-mobile-ai-bench/external/snpe/lib/dsp",
                    device_bin_path)

    # for mace
    if base_pb2.MACE == executor and abi == "armeabi-v7a":
        device.push("third_party/mace/nnlib/libhexagon_controller.so",
                    device_bin_path)

    # for tflite
    if base_pb2.TFLITE == executor:
        tflite_lib_path = ""
        if abi == "armeabi-v7a":
            tflite_lib_path = \
                "third_party/tflite/tensorflow/lite/" + \
                "lib/armeabi-v7a/libtensorflowLite.so"
        elif abi == "arm64-v8a":
            tflite_lib_path = \
                "third_party/tflite/tensorflow/lite/" + \
                "lib/arm64-v8a/libtensorflowLite.so"
        if tflite_lib_path:
            device.push(tflite_lib_path, device_bin_path)

    # for HIAI
    if base_pb2.HIAI == executor and abi == "arm64-v8a":
        hiai_lib_path = "bazel-mobile-ai-bench/external/hiai/" + \
                        "DDK/ai_ddk_mixmodel_lib/lib64/libhiai.so"
        device.push(hiai_lib_path, device_bin_path)

    # for mnn
    if base_pb2.MNN == executor:
        mnn_lib_path = ""
        if abi == "armeabi-v7a":
            mnn_lib_path = \
                "third_party/mnn/project/android/build_32/libMNN.so"
            mnn_cl_lib_path = \
                "third_party/mnn/project/android/build_32/libMNN_CL.so"
        elif abi == "arm64-v8a":
            mnn_lib_path = \
                "third_party/mnn/project/android/build_64/libMNN.so"
            mnn_cl_lib_path = \
                "third_party/mnn/project/android/build_64/libMNN_CL.so"
        if mnn_lib_path:
            device.push(mnn_lib_path, device_bin_path)
            device.push(mnn_cl_lib_path, device_bin_path)


def get_model_file(file_path, checksum, output_dir, push_list):
    filename = file_path.split('/')[-1]
    if file_path.startswith("http"):
        local_file_path = output_dir + '/' + filename
        if not os.path.exists(local_file_path) \
                or bench_utils.file_checksum(local_file_path) != checksum:
            print("downloading %s..." % filename)
            urllib.urlretrieve(file_path, local_file_path)
        aibench_check(bench_utils.file_checksum(local_file_path) == checksum,
                      "file %s md5 checksum not match" % filename)
    else:
        local_file_path = file_path
        aibench_check(bench_utils.file_checksum(local_file_path) == checksum,
                      "file %s md5 checksum not match" % filename)

    push_list.append(local_file_path)


def get_model(model_info, output_dir, push_list):
    get_model_file(model_info.model_path, model_info.model_checksum,
                   output_dir, push_list)

    if model_info.weight_path != "":
        get_model_file(model_info.weight_path, model_info.weight_checksum,
                       output_dir, push_list)


def get_proto(push_list, output_dir):
    bench_factory = aibench_pb2.BenchFactory()
    model_factory = aibench_pb2.ModelFactory()
    try:
        with open("aibench/proto/benchmark.meta", "rb") as fin:
            file_content = fin.read()
            text_format.Parse(file_content, bench_factory)
            filepath = output_dir + "/benchmark.pb"
            with open(filepath, "wb") as fout:
                fout.write(bench_factory.SerializeToString())
                push_list.append(filepath)
        with open("aibench/proto/model.meta", "rb") as fin:
            file_content = fin.read()
            text_format.Parse(file_content, model_factory)
            filepath = output_dir + "/model.pb"
            with open(filepath, "wb") as fout:
                fout.write(model_factory.SerializeToString())
                push_list.append(filepath)
    except text_format.ParseError as e:
        raise IOError("Cannot parse file.", e)

    return bench_factory, model_factory


def prepare_all_models(executors, model_names, device_types, output_dir):
    push_list = []
    bench_factory, model_factory = get_proto(push_list, output_dir)

    executors = executors.split(',') \
        if executors != "all" else base_pb2.ExecutorType.keys()
    executors = [base_pb2.ExecutorType.Value(e) for e in executors]
    model_names = model_names.split(',') \
        if model_names != "all" else base_pb2.ModelName.keys()
    model_names = [base_pb2.ModelName.Value(m) for m in model_names]
    device_types = device_types.split(',') \
        if device_types != "all" else base_pb2.DeviceType.keys()
    device_types = [base_pb2.DeviceType.Value(d) for d in device_types]

    model_infos = []
    for benchmark in bench_factory.benchmarks:
        if benchmark.executor not in executors:
            continue
        for model in benchmark.models:
            if model.model_name not in model_names:
                continue
            model_info = {
                AIBenchKeyword.executor: benchmark.executor,
                AIBenchKeyword.model_name: model.model_name,
                AIBenchKeyword.quantize: model.quantize,
            }
            downloaded = False
            for device in model.devices:
                if device not in device_types:
                    continue
                if not downloaded:
                    get_model(model, output_dir, push_list)
                    downloaded = True
                info = copy.deepcopy(model_info)
                info[AIBenchKeyword.device_type] = device
                model_infos.append(info)
    return executors, device_types, push_list, model_infos


def push_all_models(targtet_device, device_bin_path, push_list):
    for path in push_list:
        targtet_device.push(path, device_bin_path)


def prepare_datasets(configs, output_dir, input_dir):
    if input_dir.startswith("http"):
        file_path = download_file(configs, "imagenet_less.zip", output_dir)
        sh.unzip("-o", file_path, "-d", output_dir)
        return output_dir + "/imagenet_less"
    else:
        return input_dir


def push_precision_files(device, device_bin_path, input_dir):
    device.exec_command("mkdir -p %s" % device_bin_path)
    device.push("aibench/benchmark/imagenet/imagenet_blacklist.txt",
                device_bin_path)
    device.push("aibench/benchmark/imagenet/imagenet_groundtruth_labels.txt",
                device_bin_path)
    device.push("aibench/benchmark/imagenet/mobilenet_model_labels.txt",
                device_bin_path)
    if input_dir != "":
        imagenet_input_path = device_bin_path + "/inputs/"
        print("Pushing images from %s to %s ..."
              % (input_dir, imagenet_input_path))
        device.exec_command("mkdir -p %s" % imagenet_input_path)
        device.push(input_dir, imagenet_input_path, True)


def get_cpu_mask(device):
    freq_list = []
    cpu_id = 0
    cpu_mask = ''
    while True:
        try:
            freq_list.append(
                int(device.exec_command(
                           "cat /sys/devices/system/cpu/cpu%d"
                           "/cpufreq/cpuinfo_max_freq" % cpu_id)))
        except (ValueError, sh.ErrorReturnCode_1):
            break
        else:
            cpu_id += 1
    for freq in freq_list:
        cpu_mask = '1' + cpu_mask if freq == max(freq_list) else '0' + cpu_mask
    return str(hex(int(cpu_mask, 2)))[2:], cpu_mask.count('1')


def bench_run(abi,
              device,
              host_bin_path,
              bin_name,
              benchmark_option,
              input_dir,
              run_interval,
              num_threads,
              max_time_per_lock,
              benchmark_list,
              executor,
              device_types,
              device_bin_path,
              output_dir,
              dest_path,
              product_model
              ):
    i = 0
    while i < len(benchmark_list):
        print(
            "============================================================="
        )
        print("Trying to lock device %s" % device.address)
        with device_lock(device.address):
            start_time = time.time()
            print("Run on device: %s, %s, %s" %
                  (device.address, product_model, device.target_soc))
            try:
                sh.bash("tools/power.sh", device.address,
                        device.get_shell_prefix(),
                        device.target_soc, _fg=True)
            except Exception as e:
                print("Config power exception %s" % str(e))

            device.exec_command("mkdir -p %s" % device_bin_path)
            device.exec_command("rm -rf %s" %
                                os.path.join(device_bin_path,
                                             "interior"))
            device.exec_command("mkdir %s" %
                                os.path.join(device_bin_path,
                                             "interior"))
            device.exec_command("rm -rf %s" %
                                os.path.join(device_bin_path,
                                             "result.txt"))

            prepare_device_env(device, abi, device_bin_path, executor)

            if benchmark_option == base_pb2.Precision:
                push_precision_files(device, device_bin_path, input_dir)

            host_bin_full_path = "%s/%s" % (host_bin_path, bin_name)
            device_bin_full_path = "%s/%s" % (device_bin_path, bin_name)
            device.exec_command("rm -rf %s" % device_bin_full_path)
            device.push(host_bin_full_path, device_bin_path)
            print("Run %s" % device_bin_full_path)

            cpu_mask, big_core_num = get_cpu_mask(device)
            num_threads = min(big_core_num, num_threads)

            # TODO(luxuhui@xiaomi.com): opt the LIBRARY_PATH.
            cmd = "cd %s; ADSP_LIBRARY_PATH='.;/system/lib/rfsa/adsp;" \
                  "/system/vendor/lib/rfsa/adsp;/dsp';" \
                  " LD_LIBRARY_PATH=." % device_bin_path
            cmd_tflite = cmd + " taskset " + cpu_mask + " ./model_benchmark"
            cmd = cmd + " ./model_benchmark"

            elapse_minutes = 0  # run at least one model
            while elapse_minutes < max_time_per_lock \
                    and i < len(benchmark_list):
                item = benchmark_list[i]
                i += 1
                if item[AIBenchKeyword.executor] != executor or \
                        item[AIBenchKeyword.device_type] not in device_types:
                    continue
                print(
                    base_pb2.ExecutorType.Name(
                        item[AIBenchKeyword.executor]),
                    base_pb2.ModelName.Name(
                        item[AIBenchKeyword.model_name]),
                    base_pb2.DeviceType.Name(
                        item[AIBenchKeyword.device_type]),
                    "Quantized" if item[AIBenchKeyword.quantize]
                    else "Float")
                args = [
                    "--run_interval=%d" % run_interval,
                    "--num_threads=%d " % num_threads,
                    "--benchmark_option=%s" % benchmark_option,
                    "--executor=%d" % item[AIBenchKeyword.executor],
                    "--device_type=%d" % item[AIBenchKeyword.device_type],
                    "--model_name=%d" % item[AIBenchKeyword.model_name],
                    "--quantize=%s" % item[AIBenchKeyword.quantize],
                    ]
                args = ' '.join(args)
                cmd_run = cmd_tflite if item[AIBenchKeyword.executor] \
                    == base_pb2.TFLITE else cmd
                device.exec_command("%s %s" % (cmd_run, args), _fg=True)
                elapse_minutes = (time.time() - start_time) / 60
            print("Elapse time: %f minutes." % elapse_minutes)
            src_path = os.path.join(device_bin_path, "result.txt")
            tmp_path = os.path.join(output_dir, device.address + "_result.txt")
            device.pull(src_path, tmp_path)
            with open(tmp_path, "r") as tmp, open(dest_path, "a") as dest:
                dest.write(tmp.read())
        # Sleep awhile so that other pipelines can get the device lock.
        time.sleep(run_interval)
