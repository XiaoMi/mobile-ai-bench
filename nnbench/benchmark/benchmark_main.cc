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
#include "nnbench/executors/snpe/snpe_executor.h"

// define all benchmarks here
std::unique_ptr<nnbench::SnpeCPUExecutor>
    snpe_cpu_executor(new nnbench::SnpeCPUExecutor());
NNBENCH_BENCHMARK(snpe_cpu_executor.get(), InceptionV3, SNPE, CPU,
                  inception_v3.dlc, {"Mul:0"}, {"keyboard_299x299.dat"},
                  {268203});

std::unique_ptr<nnbench::SnpeGPUExecutor>
    snpe_gpu_executor(new nnbench::SnpeGPUExecutor());
NNBENCH_BENCHMARK(snpe_gpu_executor.get(), InceptionV3, SNPE, GPU,
                  inception_v3.dlc, {"Mul:0"}, {"keyboard_299x299.dat"},
                  {268203});


DEFINE_string(model_name, "all", "the model to benchmark");

int main(int argc, char **argv) {
  std::string usage = "run benchmark, e.g. " + std::string(argv[0]) +
                      " --model_name=all";
  std::cout << usage << std::endl;
  gflags::SetUsageMessage(usage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  nnbench::Status status =
      nnbench::benchmark::Benchmark::Run(FLAGS_model_name.c_str());
  return status;
}
