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

#include "aibench/benchmark/benchmark.h"

#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <vector>

namespace aibench {
namespace benchmark {

namespace {

aibench::Framework ParseFramework(const char *framework) {
  if (strcmp(framework, "MACE") == 0) {
    return aibench::Framework::MACE;
  } else if (strcmp(framework, "SNPE") == 0) {
    return aibench::Framework::SNPE;
  } else if (strcmp(framework, "NCNN") == 0) {
    return aibench::Framework::NCNN;
  } else if (strcmp(framework, "TFLITE") == 0) {
    return aibench::Framework::TFLITE;
  } else {
    return aibench::Framework::MACE;
  }
}

aibench::Runtime ParseRuntime(const char *runtime) {
  if (strcmp(runtime, "CPU") == 0) {
    return aibench::Runtime::CPU;
  } else if (strcmp(runtime, "GPU") == 0) {
    return aibench::Runtime::GPU;
  } else if (strcmp(runtime, "DSP") == 0) {
    return aibench::Runtime::DSP;
  } else {
    return aibench::Runtime::CPU;
  }
}

}  // namespace

static std::vector<Benchmark *> *all_benchmarks = nullptr;

Benchmark::Benchmark(BaseExecutor *executor,
                     const char *model_name,
                     const char *model_file,
                     std::vector<std::string> input_names,
                     std::vector<std::string> input_files,
                     std::vector<std::vector<int64_t>> input_shapes,
                     std::vector<std::string> output_names,
                     std::vector<std::vector<int64_t>> output_shapes)
    : executor_(executor),
      model_name_(model_name),
      model_file_(model_file),
      input_names_(input_names),
      input_files_(input_files),
      input_shapes_(input_shapes),
      output_names_(output_names),
      output_shapes_(output_shapes) {
  if (input_names.size() != input_shapes.size() ||
      (input_files.size() != input_shapes.size() && input_files.size() > 0) ||
      output_names.size() != output_shapes.size()) {
    printf(
        "size of input_names(%d), input_files(%d) and input_shapes(%d) "
        "should be equal. sizeof output_names(%d) and output_shapes(%d) "
        "should be equal.\n",
        static_cast<int>(input_names.size()),
        static_cast<int>(input_files.size()),
        static_cast<int>(input_shapes.size()),
        static_cast<int>(output_names.size()),
        static_cast<int>(output_shapes.size()));
    abort();
  }
  Register();
}

// Run all benchmarks filtered by model_name
Status Benchmark::Run(const char *model_name,
                      const char *framework,
                      const char *runtime,
                      int run_interval,
                      int num_threads) {
  if (!all_benchmarks) return SUCCESS;

  // sort by model name, framework and runtime
  // the compare function tends to shuffle benchmarks by runtime
  std::sort(all_benchmarks->begin(), all_benchmarks->end(),
            [](const Benchmark *lhs, const Benchmark *rhs) {
              return lhs->model_name_ < rhs->model_name_ ||
                     (lhs->model_name_ == rhs->model_name_ &&
                      (lhs->executor_->GetFramework() <
                           rhs->executor_->GetFramework() ||
                       (lhs->executor_->GetFramework() ==
                            rhs->executor_->GetFramework() &&
                        lhs->executor_->GetRuntime() != aibench::CPU)));
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

    // sleep run_interval seconds to cool off the target
    printf("sleep %d\n", run_interval);
    sleep(static_cast<uint32_t>(run_interval));

    double init_ms, run_ms;
    printf("benchmarking: %s,%d,%d\n", b->model_name_.c_str(),
           b->executor_->GetFramework(), b->executor_->GetRuntime());
    Status status = b->Run(&init_ms, &run_ms, num_threads);
    if (status != SUCCESS) {
      res = status;
      printf("benchmark failed: %s,%d,%d\n", b->model_name_.c_str(),
             b->executor_->GetFramework(), b->executor_->GetRuntime());
      continue;
    }
    // model_name,framework,runtime,init time,inference time
    printf("benchmark: %s,%d,%d,%.3f,%.3f\n", b->model_name_.c_str(),
           b->executor_->GetFramework(), b->executor_->GetRuntime(), init_ms,
           run_ms);
  }
  return res;
}

void Benchmark::Register() {
  if (!all_benchmarks) all_benchmarks = new std::vector<Benchmark *>;
  all_benchmarks->push_back(this);
}

Status Benchmark::Run(double *init_ms, double *run_ms, int num_threads) {
  static const int64_t kMinIters = 5;
  static const int64_t kMaxIters = 20;
  static const double kMinTime = 2000000;  // microseconds
  static const float quantile = 0.8;
  int64_t start_time, end_time;
  Status status;
  // Init the target's environment
  status = executor_->Init(model_file_.c_str(), num_threads);
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }
  // prepare
  start_time = NowMicros();
  status = executor_->Prepare(model_file_.c_str());
  end_time = NowMicros();
  *init_ms = (end_time - start_time) * 1e-3;
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }
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
  for (size_t i = 0; i < output_names_.size(); ++i) {
    int64_t output_size =
        std::accumulate(output_shapes_[i].begin(), output_shapes_[i].end(), 1,
                        std::multiplies<int64_t>());
    auto buffer_out = std::shared_ptr<float>(new float[output_size],
                                             std::default_delete<float[]>());
    outputs[output_names_[i]] = BaseTensor(output_shapes_[i], buffer_out);
  }

  for (int i = 0; i < 2; ++i) {
    status = executor_->Run(inputs, &outputs);
  }
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }

  std::vector<int64_t> durations;
  int64_t total_duration = 0;
  size_t benchmark_iters = 0;

  for (int i = 0; i < kMinIters || (total_duration < kMinTime && i < kMaxIters);
       ++i) {
    start_time = NowMicros();
    status = executor_->Run(inputs, &outputs);
    end_time = NowMicros();
    durations.push_back(end_time - start_time);
    total_duration += durations.back();
    if (status != SUCCESS) {
      executor_->Finish();
      return status;
    }
    ++benchmark_iters;
  }

  std::sort(durations.begin(), durations.end());

  size_t valid_iters = std::max(
      static_cast<size_t>(1), static_cast<size_t>(benchmark_iters * quantile));
  size_t start_iter = (benchmark_iters - valid_iters) / 2;
  valid_iters = std::min(valid_iters, benchmark_iters - start_iter);
  total_duration =
      std::accumulate(durations.begin() + start_iter,
                      durations.begin() + (start_iter + valid_iters), 0);

  *run_ms = total_duration * 1e-3 / valid_iters;
  executor_->Finish();
  return SUCCESS;
}

int64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

}  // namespace benchmark
}  // namespace aibench
