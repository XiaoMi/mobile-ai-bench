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
from sh_commands import FRAMEWORKS, RUNTIMES

abi_types = [
    "armeabi-v7a",
    "arm64-v8a",
    "host",
]


def add_statistics(statistics, all_statistics):
    statistics.sort(key=lambda record: "".join(str(x) for x in record))
    i = 0
    while i < len(statistics):
        data_str = ",".join(statistics[i][:-2])
        model_index = i
        for framework in range(len(FRAMEWORKS)):
            data_str += ","
            if i < len(statistics) \
                    and statistics[model_index][0] == statistics[i][0] \
                    and framework == statistics[i][-2]:
                data_str += statistics[i][-1]
                i += 1
        data_str += "\n"
        all_statistics.append(data_str)


def report_run_statistics(stdout,
                          abi,
                          serialno,
                          all_prepare,
                          all_run_avg):
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

    prepare = []
    run_avg = []
    for metric in metrics:
        record = [metric[0], device_name, target_soc, abi,
                  RUNTIMES[int(metric[2])], metric[1]]
        prepare.append(record + [metric[3]])
        run_avg.append(record + [metric[4]])

    add_statistics(prepare, all_prepare)
    add_statistics(run_avg, all_run_avg)


def write_statistics(f, title, statistics):
    f.write(title)
    header = "model_name,device_name,soc,abi,runtime,%s\n" \
             % ",".join(FRAMEWORKS)
    f.write(header)
    statistics.sort()
    f.write("".join(statistics))


def write_all_statistics(all_prepare, all_run_avg, output_dir):
    report_filename = output_dir + "/report.csv"
    with open(report_filename, 'w') as f:
        write_statistics(f, "Prepare:\n", all_prepare)
        write_statistics(f, "Run_avg:\n", all_run_avg)


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
        "--model_names",
        type=str,
        default="all",
        help="models to run")
    parser.add_argument(
        "--frameworks",
        type=str,
        default="all",
        help="frameworks to run, MACE/SNPE/NCNN/TENSORFLOW_LITE,"
             "comma seperated list or all")
    parser.add_argument(
        "--runtimes",
        type=str,
        default="all",
        help="runtimes to run, CPU/GPU/DSP, comma seperated list or all")
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
    frameworks = FLAGS.frameworks.split(',') \
        if FLAGS.frameworks != "all" else list(FRAMEWORKS)
    runtimes = FLAGS.runtimes.split(',')
    model_names = FLAGS.model_names.split(',')
    target = FLAGS.target
    host_bin_path, bin_name = sh_commands.bazel_target_to_bin(target)
    if "MACE" in frameworks:
        sh_commands.build_mace(FLAGS.target_abis, FLAGS.output_dir)
    all_prepare = []
    all_run_avg = []
    for target_abi in target_abis:
        if target_abi not in abi_types:
            print("Not supported abi: %s" % target_abi)
            continue
        sh_commands.bazel_build(target, target_abi, frameworks)
        if target_abi == "host":
            print("Unable to run target on host yet!")
            continue
        for serialno in target_devices:
            if target_abi not in set(
                    sh_commands.adb_supported_abis(serialno)):
                print("Skip device %s which does not support ABI %s" %
                      (serialno, target_abi))
                continue
            stdouts = sh_commands.adb_run(target_abi, serialno, host_bin_path,
                                          bin_name, frameworks,
                                          model_names, runtimes,
                                          output_dir=FLAGS.output_dir)
            report_run_statistics(stdouts, target_abi, serialno,
                                  all_prepare, all_run_avg)
    write_all_statistics(all_prepare, all_run_avg, FLAGS.output_dir)


if __name__ == "__main__":
    FLAGS, unparsed = parse_args()
    main(unused_args=[sys.argv[0]] + unparsed)
