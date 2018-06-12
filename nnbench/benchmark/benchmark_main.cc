// Copyright 2018 Xiaomi, Inc.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <iostream>

#include "gflags/gflags.h"
#include "nnbench/benchmark/benchmark.h"
#include "nnbench/executors/base_executor.h"
#include "nnbench/executors/ncnn/ncnn_executor.h"
#include "nnbench/executors/snpe/snpe_executor.h"

// define all benchmarks here
std::unique_ptr<nnbench::SnpeCPUExecutor>
    snpe_cpu_executor(new nnbench::SnpeCPUExecutor());
NNBENCH_BENCHMARK(snpe_cpu_executor.get(), InceptionV3, SNPE, CPU,
                  inception_v3.dlc, (std::vector<std::string>{"Mul:0"}),
                  (std::vector<std::string>{"keyboard_299x299.dat"}),
                  (std::vector<std::vector<int64_t>>{{299, 299, 3}}));
NNBENCH_BENCHMARK(snpe_cpu_executor.get(), VGG16, SNPE, CPU,
                  vgg16.dlc, (std::vector<std::string>{"input:0"}),
                  (std::vector<std::string>{"chairs_224x224.raw"}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));

std::unique_ptr<nnbench::SnpeGPUExecutor>
    snpe_gpu_executor(new nnbench::SnpeGPUExecutor());
NNBENCH_BENCHMARK(snpe_gpu_executor.get(), InceptionV3, SNPE, GPU,
                  inception_v3.dlc, (std::vector<std::string>{"Mul:0"}),
                  (std::vector<std::string>{"keyboard_299x299.dat"}),
                  (std::vector<std::vector<int64_t>>{{299, 299, 3}}));
// TODO(wuchenghui): benchmark snpe + gpu + vgg16

std::unique_ptr<nnbench::SnpeDSPExecutor>
    snpe_dsp_executor(new nnbench::SnpeDSPExecutor());
NNBENCH_BENCHMARK(snpe_dsp_executor.get(), InceptionV3, SNPE, DSP,
                  inception_v3_quantized.dlc,
                  (std::vector<std::string>{"Mul:0"}),
                  (std::vector<std::string>{"keyboard_299x299.dat"}),
                  (std::vector<std::vector<int64_t>>{{299, 299, 3}}));
NNBENCH_BENCHMARK(snpe_dsp_executor.get(), VGG16, SNPE, DSP,
                  vgg16_quantized.dlc, (std::vector<std::string>{"input:0"}),
                  (std::vector<std::string>{"chairs_224x224.raw"}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));

std::unique_ptr<nnbench::NcnnExecutor>
    ncnn_executor(new nnbench::NcnnExecutor());
NNBENCH_BENCHMARK(ncnn_executor.get(), AlexNet, NCNN, CPU,
                  alexnet.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{227, 227, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), GoogLeNet, NCNN, CPU,
                  googlenet.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), MobileNetV1, NCNN, CPU,
                  mobilenet.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), MobileNetV2, NCNN, CPU,
                  mobilenet_v2.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), ResNet18, NCNN, CPU,
                  resnet18.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), SqueezeNet, NCNN, CPU,
                  squeezenet.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{227, 227, 3}}));
NNBENCH_BENCHMARK(ncnn_executor.get(), VGG16, NCNN, CPU,
                  vgg16.param, (std::vector<std::string>{"data"}),
                  (std::vector<std::string>{}),
                  (std::vector<std::vector<int64_t>>{{224, 224, 3}}));

DEFINE_string(model_name, "all", "the model to benchmark");
DEFINE_string(framework, "all", "the framework to benchmark");
DEFINE_string(runtime, "all", "the runtime to benchmark");

int main(int argc, char **argv) {
  std::string usage = "run benchmark, e.g. " + std::string(argv[0]) +
                      " --model_name=all";
  gflags::SetUsageMessage(usage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  nnbench::Status status = nnbench::benchmark::Benchmark::Run(
      FLAGS_model_name.c_str(), FLAGS_framework.c_str(), FLAGS_runtime.c_str());
  return status;
}
