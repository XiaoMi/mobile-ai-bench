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

#include "nnbench/benchmark/benchmark.h"

#include <string.h>
#include <sys/time.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <vector>
#include <map>

namespace nnbench {
namespace benchmark {

namespace {

nnbench::Framework ParseFramework(const char *framework) {
  if (strcmp(framework, "MACE") == 0) {
    return nnbench::Framework::MACE;
  } else if (strcmp(framework, "SNPE") == 0) {
    return nnbench::Framework::SNPE;
  } else if (strcmp(framework, "NCNN") == 0) {
    return nnbench::Framework::NCNN;
  } else if (strcmp(framework, "TENSORFLOW_LITE") == 0) {
    return nnbench::Framework::TENSORFLOW_LITE;
  } else {
    return nnbench::Framework::MACE;
  }
}

nnbench::Runtime ParseRuntime(const char *runtime) {
  if (strcmp(runtime, "CPU") == 0) {
    return nnbench::Runtime::CPU;
  } else if (strcmp(runtime, "GPU") == 0) {
    return nnbench::Runtime::GPU;
  } else if (strcmp(runtime, "DSP") == 0) {
    return nnbench::Runtime::DSP;
  } else {
    return nnbench::Runtime::CPU;
  }
}

}  // namespace

static std::vector<Benchmark *> *all_benchmarks = nullptr;

Benchmark::Benchmark(BaseExecutor *executor,
                     const char *model_name,
                     const char *model_file,
                     std::vector<std::string> input_names,
                     std::vector<std::string> input_files,
                     std::vector<std::vector<int64_t>> input_shapes)
    : executor_(executor),
      model_name_(model_name),
      model_file_(model_file),
      input_names_(input_names),
      input_files_(input_files),
      input_shapes_(input_shapes) {
  if (input_names.size() != input_shapes_.size() ||
      (input_files.size() != input_shapes_.size() && input_files.size() > 0)) {
    printf("size of input_names(%d), input_files(%d) and input_shapes(%d) "
               "should be equal\n",
           static_cast<int>(input_names.size()),
           static_cast<int>(input_files.size()),
           static_cast<int>(input_shapes_.size()));
    abort();
  }
  Register();
}

// Run all benchmarks filtered by model_name
Status Benchmark::Run(const char *model_name, const char *framework,
                      const char *runtime) {
  if (!all_benchmarks) return SUCCESS;

  // sort by model name
  std::sort(all_benchmarks->begin(), all_benchmarks->end(),
            [](const Benchmark *lhs, const Benchmark *rhs) {
              return lhs->model_name_ < rhs->model_name_;
            });

  // Internal perf regression tools depends on the output formatting,
  // please keep in consistent when modifying
  Status res = SUCCESS;
  for (auto b : *all_benchmarks) {
    if (strcmp(model_name, "all") != 0 &&
        strcmp(model_name, b->model_name_.c_str()) != 0)
      continue;
    if (strcmp(framework, "all") != 0 &&
        ParseFramework(framework) != b->executor_->GetFramework())
      continue;
    if (strcmp(runtime, "all") != 0 &&
        ParseRuntime(runtime) != b->executor_->GetRuntime())
      continue;
    double init_seconds, run_seconds;
    Status status = b->Run(&init_seconds, &run_seconds);
    if (status != SUCCESS) {
      res = status;
      continue;
    }
    // model_name,framework,runtime,init time,inference time
    printf("benchmark:%s,%d,%d,%f,%f\n",
           b->model_name_.c_str(),
           b->executor_->GetFramework(),
           b->executor_->GetRuntime(),
           init_seconds,
           run_seconds);
  }
  return res;
}

void Benchmark::Register() {
  if (!all_benchmarks) all_benchmarks = new std::vector<Benchmark *>;
  all_benchmarks->push_back(this);
}

Status Benchmark::Run(double *init_seconds, double *run_seconds) {
  static const int64_t kMinIters = 10;
  static const int64_t kMaxIters = 1000000000;
  static const double kMinTime = 1;
  int64_t iters = kMinIters;
  int64_t start_time, end_time;
  // prepare
  start_time = NowMicros();
  Status status = executor_->Prepare(model_file_.c_str());
  end_time = NowMicros();
  *init_seconds = (end_time - start_time) * 1e-6;
  if (status != SUCCESS) return status;
  // warm-up
  std::map<std::string, BaseTensor> inputs;
  std::map<std::string, BaseTensor> outputs;
  for (size_t i = 0; i < input_names_.size(); ++i) {
    int64_t input_size =
        std::accumulate(input_shapes_[i].begin(), input_shapes_[i].end(), 1,
                        std::multiplies<int64_t>());
    std::shared_ptr<float> input_data(new float[input_size]);
    if (input_files_.size() > 0 && !input_files_[i].empty()) {
      std::ifstream fin(input_files_[i].c_str(),
                        std::ios::in | std::ios::binary);
      if (!fin) {
        printf("Open file %s failed!\n", input_files_[i].c_str());
        abort();
      }
      fin.read(reinterpret_cast<char *>(input_data.get()),
               input_size * sizeof(float));
      fin.close();
    } else {
      // random input
      unsigned int seed = 1;
      for (size_t j = 0; j < input_size; ++j)
        input_data.get()[j] = rand_r(&seed) % 100 / 100.0f;
    }
    BaseTensor input_tensor = BaseTensor(input_shapes_[i], input_data);
    inputs.insert({input_names_[i], input_tensor});
  }
  for (int i = 0; i < 5; ++i) {
    status = executor_->Run(inputs, &outputs);
  }
  if (status != SUCCESS) return status;
  while (true) {
    start_time = NowMicros();
    for (int i = 0; i < iters; ++i) {
      executor_->Run(inputs, &outputs);
    }
    end_time = NowMicros();
    const double seconds = (end_time - start_time) * 1e-6;
    if (seconds >= kMinTime || iters >= kMaxIters) {
      *run_seconds = seconds / iters;
      return SUCCESS;
    }

    // Update number of iterations.
    // Overshoot by 100% in an attempt to succeed the next time.
    double multiplier = 2.0 * kMinTime / std::max(seconds, 1e-9);
    iters = std::min<int64_t>(multiplier * iters, kMaxIters);
  }
}

int64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

}  // namespace benchmark
}  // namespace nnbench
