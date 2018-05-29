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
#include <vector>
#include <map>

namespace nnbench {
namespace testing {

static std::vector<Benchmark *> *all_benchmarks = nullptr;

Benchmark::Benchmark(BaseExecutor *executor, const char *model_name)
    : model_name_(model_name), executor_(executor) {
  Register();
}

// Run all benchmarks filtered by model_name
Status Benchmark::Run(const char *model_name) {
  if (!all_benchmarks) return SUCCESS;

  // sort by model name
  std::sort(all_benchmarks->begin(), all_benchmarks->end(),
            [](const Benchmark *lhs, const Benchmark *rhs) {
              return lhs->model_name_ < rhs->model_name_;
            });

  // Internal perf regression tools depends on the output formatting,
  // please keep in consistent when modifying
  Status res = SUCCESS;
  printf("model_name,framework,runtime,init time,inference time\n");
  for (auto b : *all_benchmarks) {
    if (strcmp(model_name, "all") != 0 &&
        strcmp(model_name, b->model_name_.c_str()) != 0) continue;
    double init_seconds, run_seconds;
    Status status = b->Run(&init_seconds, &run_seconds);
    if (status != SUCCESS) {
      res = status;
      continue;
    }
    // TODO(wuchenghui): complete benchmark info
    printf("%s,%d,%d,%f,%f\n",
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
  Status status = executor_->Prepare(model_name_.c_str());
  end_time = NowMicros();
  *init_seconds = (end_time - start_time) * 1e-6;
  if (status != SUCCESS) return status;
  // warm-up
  std::map<std::string, BaseTensor> inputs;
  std::map<std::string, BaseTensor> outputs;
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

}  // namespace testing
}  // namespace nnbench
