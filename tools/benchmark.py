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

import argparse
import os
import sys

import sh_commands

abi_types = [
    "armeabi-v7a",
    "arm64-v8a",
    "x86_64",
]


FRAMEWORKS = (
    "MACE",
    "SNPE",
    "NCNN",
    "TENSORFLOW_LITE"
)


RUNTIMES = (
    "CPU",
    "GPU",
    "DSP"
)


def report_run_statistics(stdout,
                          abi,
                          serialno,
                          output_dir):
    metrics = []
    for line in stdout.split('\n'):
        if line.startswith("benchmark:"):
            parts = line[10:].split(',')
            if len(parts) == 5:
                parts[1] = int(parts[1])  # framework
                parts[2] = int(parts[2])  # runtime
                parts[3] = str(float(parts[3]))  # prepare
                parts[4] = str(float(parts[4]))  # run_avg
                metrics.append(parts)

    props = sh_commands.adb_getprop_by_serialno(serialno)
    device_name = props.get("ro.product.model", "")
    target_soc = props.get("ro.board.platform", "")

    report_filename = output_dir + "/report.csv"
    if not os.path.exists(report_filename):
        with open(report_filename, 'w') as f:
            f.write("model_name,framework,device_name,soc,abi,runtime,"
                    "prepare(s),run_avg(s)\n")

    with open(report_filename, 'a') as f:
        for metric in metrics:
            data_str = "{model_name},{framework},{device_name},{soc},{abi}," \
                       "{runtime},{prepare},{run_avg}\n" \
                .format(model_name=metric[0],
                        framework=FRAMEWORKS[metric[1]],
                        device_name=device_name,
                        soc=target_soc,
                        abi=abi,
                        runtime=RUNTIMES[metric[2]],
                        prepare=metric[3],
                        run_avg=metric[4],
                        )
            f.write(data_str)


def parse_args():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--target_abis",
        type=str,
        default="armeabi-v7a",
        help="Target ABIs, comma seperated list")
    parser.add_argument(
        "--target_socs",
        type=str,
        default="all",
        help="SoCs (ro.board.platform from getprop) to build, "
             "comma seperated list or all/random")
    parser.add_argument(
        "--args",
        type=str,
        default="",
        help="Command args")
    parser.add_argument(
        "--target",
        type=str,
        default="//nnbench/benchmark:model_benchmark",
        help="Bazel target to build")
    parser.add_argument(
        "--output_dir",
        type=str,
        default="output",
        help="benchmark output directory")
    return parser.parse_known_args()


def main(unused_args):
    target_socs = None
    if FLAGS.target_socs != "all":
        target_socs = set(FLAGS.target_socs.split(','))
    target_devices = sh_commands.get_target_socs_serialnos(target_socs)
    if not target_devices:
        print("No available device!")
    if not os.path.exists(FLAGS.output_dir):
        os.mkdir(FLAGS.output_dir)

    target_abis = FLAGS.target_abis.split(',')
    target = FLAGS.target
    host_bin_path, bin_name = sh_commands.bazel_target_to_bin(target)
    for target_abi in target_abis:
        if target_abi not in abi_types:
            print("Not supported abi: %s" % target_abi)
            continue
        sh_commands.bazel_build(target, target_abi)
        if target_abi == "x86_64":
            print("Unable to run target on host yet!")
            continue
        for serialno in target_devices:
            if target_abi not in set(
                    sh_commands.adb_supported_abis(serialno)):
                print("Skip device %s which does not support ABI %s" %
                      (serialno, target_abi))
                continue
            stdouts = sh_commands.adb_run(target_abi, serialno, host_bin_path,
                                          bin_name, FLAGS.args,
                                          output_dir=FLAGS.output_dir)
            report_run_statistics(stdouts, target_abi, serialno,
                                  FLAGS.output_dir)

if __name__ == "__main__":
    FLAGS, unparsed = parse_args()
    main(unused_args=[sys.argv[0]] + unparsed)
