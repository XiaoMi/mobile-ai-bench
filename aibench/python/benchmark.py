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

import argparse
import os
import random
import sh
import sys
import threading
import yaml

from aibench.proto import base_pb2
from aibench.python.utils.common import ABI_TYPES
from aibench.python.utils.common import aibench_check
import bench_engine


class ResultProcessorUtil(object):

    @staticmethod
    def format_with_executor(records):
        records.sort(key=lambda record: "".join(str(x) for x in record))
        formatted_records = []
        i = 0
        while i < len(records):
            data_str = ",".join(records[i][:-2])
            model_index = i
            for executor in base_pb2.ExecutorType.values():
                data_str += ","
                if i < len(records) \
                        and records[model_index][:-2] == records[i][:-2] \
                        and executor == records[i][-2]:
                    data_str += records[i][-1]
                    i += 1
            formatted_records.append(data_str)
        return formatted_records

    @staticmethod
    def write_records(records, filename):
        header = "model_name,quantize,product,soc,abi,device,%s\n" \
                 % ",".join(base_pb2.ExecutorType.keys())
        records = ResultProcessorUtil.format_with_executor(records)
        report = os.path.join(FLAGS.output_dir, filename)
        with open(report, 'w') as f:
            f.write(header + "\n".join(records))
        print("Write report file %s." % report)


class ResultProcessor(object):

    def process(self, lines, product_info):
        raise NotImplementedError("process")

    def report(self):
        raise NotImplementedError("report")


class PerformanceProcessor(ResultProcessor):
    def __init__(self):
        self.prepares = []
        self.run_avgs = []

    def process(self, lines, product_info):
        head = str(base_pb2.Performance) + ":"
        for line in lines:
            if line.startswith(head):
                # executor,model_name,device_type,quantize,prepare,run_avg
                parts = line[len(head):].split(',')

                executor = int(parts[0])
                model_name = base_pb2.ModelName.Name(int(parts[1]))
                device_type = base_pb2.DeviceType.Name(int(parts[2]))
                quantize = "Quantized" if int(parts[3]) else "Float"
                prepare = str(float(parts[4]))
                run_avg = str(float(parts[5]))

                record = [model_name, quantize] + product_info + [
                    device_type, executor]
                self.prepares.append(record + [prepare])
                self.run_avgs.append(record + [run_avg])

    def report(self):
        ResultProcessorUtil.write_records(self.prepares, "prepare_report.csv")
        ResultProcessorUtil.write_records(self.run_avgs, "run_report.csv")


class PrecisionProcessor(ResultProcessor):
    def __init__(self):
        self.precisions = []

    def process(self, lines, product_info):
        head = str(base_pb2.Precision) + ":"
        for line in lines:
            if line.startswith(head):
                # executor,model_name,device_type,quantize,evaluator,precision
                parts = line[len(head):].split(',')

                executor = int(parts[0])
                model_name = base_pb2.ModelName.Name(int(parts[1]))
                device_type = base_pb2.DeviceType.Name(int(parts[2]))
                quantize = "Quantized" if int(parts[3]) else "Float"
                evaluator = int(parts[4])
                precision = str(float(parts[5]))

                aibench_check(
                    evaluator == base_pb2.MetricEvaluator.ImageClassification,
                    "Only support ImageClassification now")
                record = [model_name, quantize] + product_info + [
                    device_type, executor]
                self.precisions.append(record + [precision])

    def report(self):
        ResultProcessorUtil.write_records(self.precisions,
                                          "precision_report.csv")


def process_result(result_files):
    if FLAGS.benchmark_option == \
            base_pb2.BenchmarkOption.Name(base_pb2.Performance):
        result_processor = PerformanceProcessor()
    else:
        result_processor = PrecisionProcessor()

    for result_file in result_files:
        file_name = os.path.basename(result_file)
        product_info = file_name.split('_')[:-1]
        with open(result_file) as f:
            lines = f.readlines()
            result_processor.process(lines, product_info)

    result_processor.report()


def get_configs(config_file="tools/configs.yml"):
    with open(config_file) as f:
        return yaml.load(f)


def parse_args():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--benchmark_option",
        type=str,
        default="Performance",
        help="Benchmark options, Performace/Precision")
    parser.add_argument(
        "--input_dir",
        type=str,
        default="",
        help="Input data directory for precision benchmark")
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
        "--executors",
        type=str,
        default="all",
        help="executors to run, MACE/SNPE/NCNN/TFLITE/HIAI,"
             "comma seperated list or all")
    parser.add_argument(
        "--device_types",
        type=str,
        default="all",
        help="device_types to run, CPU/GPU/DSP/NPU, "
             "comma seperated list or all")
    parser.add_argument(
        "--run_interval",
        type=int,
        default=10,
        help="run interval between benchmarks, seconds")
    parser.add_argument(
        "--num_threads",
        type=int,
        default=4,
        help="number of threads")
    parser.add_argument(
        "--num_targets",
        type=int,
        default=0,
        help="number of targets to benchmark, default all connected targets.")
    parser.add_argument(
        "--args",
        type=str,
        default="",
        help="Command args")
    parser.add_argument(
        "--target",
        type=str,
        default="//aibench/benchmark:model_benchmark",
        help="Bazel target to build")
    parser.add_argument(
        "--output_dir",
        type=str,
        default="output",
        help="benchmark output directory")
    parser.add_argument(
        "--max_time_per_lock",
        type=int,
        default=300,
        help="Max run time(in minutes) after every device lock for other "
             "pipelines to get device lock.")
    parser.add_argument(
        "--all_devices_at_once",
        action="store_true",
        help="whether to benchmark all devices simultaneously.")
    return parser.parse_known_args()


def run_on_device(target_device,
                  target_abi,
                  push_list,
                  executors,
                  target,
                  device_types,
                  host_bin_path,
                  bin_name,
                  input_dir,
                  benchmark_option,
                  benchmark_list,
                  result_files,
                  ):
    props = target_device.get_props()
    product_model = props["ro.product.model"]
    result_path = os.path.join(
        FLAGS.output_dir, product_model + "_" + target_device.target_soc
        + "_" + target_abi + "_" + "result.txt")
    sh.rm("-rf", result_path)
    device_bench_path = target_device.get_bench_path()
    target_device.exec_command("mkdir -p %s" % device_bench_path)
    bench_engine.push_all_models(target_device, device_bench_path, push_list)
    for executor in executors:
        avail_device_types = \
            target_device.get_available_device_types(device_types, target_abi,
                                                     executor)
        bench_engine.bazel_build(target, target_abi, executor,
                                 avail_device_types)
        bench_engine.bench_run(
            target_abi, target_device, host_bin_path, bin_name,
            benchmark_option, input_dir, FLAGS.run_interval,
            FLAGS.num_threads, FLAGS.max_time_per_lock,
            benchmark_list, executor, avail_device_types, device_bench_path,
            FLAGS.output_dir, result_path, product_model)
    result_files.append(result_path)


def main(unused_args):
    aibench_check(FLAGS.benchmark_option in base_pb2.BenchmarkOption.keys(),
                  "Wrong benchmark option %s" % FLAGS.benchmark_option)
    benchmark_option = base_pb2.BenchmarkOption.Value(FLAGS.benchmark_option)
    target_socs = None
    if FLAGS.target_socs != "all":
        target_socs = set(FLAGS.target_socs.split(','))

    target_devices = bench_engine.get_target_devices_by_socs(target_socs)
    if not target_devices:
        print("No available target!")
    if FLAGS.num_targets != 0 and FLAGS.num_targets < len(target_devices):
        random.shuffle(target_devices)
        target_devices = target_devices[:FLAGS.num_targets]

    if not os.path.exists(FLAGS.output_dir):
        os.mkdir(FLAGS.output_dir)

    target_abis = FLAGS.target_abis.split(',')

    target = FLAGS.target
    host_bin_path, bin_name = bench_engine.bazel_target_to_bin(target)

    executors, device_types, push_list, benchmark_list = \
        bench_engine.prepare_all_models(
            FLAGS.executors, FLAGS.model_names, FLAGS.device_types,
            FLAGS.output_dir)

    configs = get_configs()
    input_dir = bench_engine.prepare_datasets(configs, FLAGS.output_dir,
                                              FLAGS.input_dir)

    if base_pb2.TFLITE in executors:
        bench_engine.get_tflite(configs, FLAGS.output_dir)
    if base_pb2.MNN in executors:
        bench_engine.get_mnn(configs, FLAGS.output_dir)

    result_files = []
    for target_abi in target_abis:
        print("Prepare to run models on %s" % target_abi)
        if target_abi not in ABI_TYPES:
            print("Not supported abi: %s" % target_abi)
            continue
        if target_abi == "host":
            print("Unable to run on host yet!")
            continue

        threads = []
        for target_device in target_devices:
            if target_abi not in target_device.target_abis:
                print("Skip device %s which does not support ABI %s" %
                      (target_device.address, target_abi))
                continue

            avail_executors = \
                target_device.get_available_executors(executors, target_abi)
            if len(avail_executors) == 0:
                print("Skip device %s which doesn't support current "
                      "executors" % target_device.address)
                continue

            if FLAGS.all_devices_at_once:
                t = threading.Thread(
                    target=run_on_device,
                    args=(target_device, target_abi, push_list,
                          avail_executors, target, device_types,
                          host_bin_path, bin_name, input_dir, benchmark_option,
                          benchmark_list, result_files,))
                t.start()
                threads.append(t)
            else:
                run_on_device(
                    target_device, target_abi, push_list, avail_executors,
                    target, device_types, host_bin_path, bin_name, input_dir,
                    benchmark_option, benchmark_list, result_files,)

        if FLAGS.all_devices_at_once:
            for t in threads:
                t.join()
    if len(result_files) > 0:
        process_result(result_files)


if __name__ == "__main__":
    FLAGS, unparsed = parse_args()
    main(unused_args=[sys.argv[0]] + unparsed)
