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

#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <map>
#include <vector>

#include "mace/utils/logging.h"

namespace aibench {
namespace benchmark {

int64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

Benchmark::Benchmark(BaseExecutor *executor,
                     const ModelName &model_name,
                     const bool quantize,
                     const std::vector<std::string> &input_names,
                     const std::vector<std::vector<int64_t>> &input_shapes,
                     const std::vector<std::string> &output_names,
                     const std::vector<std::vector<int64_t>> &output_shapes,
                     const int run_interval,
                     const int num_threads)
    : executor_(executor),
      model_name_(model_name),
      quantize_(quantize),
      input_names_(input_names),
      input_shapes_(input_shapes),
      output_names_(output_names),
      output_shapes_(output_shapes),
      run_interval_(run_interval),
      num_threads_(num_threads) {
  MACE_CHECK(
      ((input_names.size() == input_shapes.size()) &&
      (output_names.size() == output_shapes.size()
          || output_names.size() == 0)),
      "Size of input_names(", input_names.size(),
      ") and input_shapes(", input_shapes.size(),
      ") should be equal. sizeof output_names(", output_names.size(),
      ") and output_shapes(", output_shapes.size(), ") "
      "should be equal.");
}

std::string Benchmark::GetBenchmarkInfo() const {
  std::stringstream stream;
  stream << executor_->GetExecutorType() << "," << model_name_ << ","
         << executor_->GetDeviceType() << "," << quantize_ << ",";
  return stream.str();
}

Status Benchmark::LogResult(const std::string &result) {
  std::ofstream out_file("result.txt", std::ofstream::out | std::ofstream::app);
  if (out_file.is_open()) {
    out_file << result << std::endl;
    out_file.close();
  } else {
    LOG(FATAL) << "Failed to log results to result.txt.";
  }

  return SUCCESS;
}

Status PerformanceBenchmark::Run() {
  // sleep run_interval seconds to cool off the target
  LOG(INFO) << "sleep " << run_interval_ << "s";
  sleep(static_cast<uint32_t>(run_interval_));

  double init_ms, run_ms;
  LOG(INFO) << "benchmarking: " << GetBenchmarkInfo();

  Status status = Run(&init_ms, &run_ms);

  if (status != SUCCESS) {
    LOG(WARNING) << "benchmark failed: " << GetBenchmarkInfo();
  } else {
    // executor,model_name,device_type,quantize,init time,inference time
    std::stringstream stream;
    stream << BenchmarkOption::Performance << ":" << GetBenchmarkInfo()
           << std::fixed << std::setprecision(3) << init_ms << "," << run_ms;
    LOG(INFO) << stream.str();
    LogResult(stream.str());
  }
  return status;
}

Status PerformanceBenchmark::Run(double *init_ms, double *run_ms) {
  static const int64_t kMinIters = 5;
  static const int64_t kMaxIters = 20;
  static const double kMinTime = 2000000;  // microseconds
  static const float quantile = 0.8;
  int64_t start_time, end_time;
  Status status;
  // Init the target's environment
  status = executor_->Init(num_threads_);
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }
  // prepare
  start_time = NowMicros();
  status = executor_->Prepare();
  end_time = NowMicros();
  *init_ms = (end_time - start_time) * 1e-3;
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }

  std::map<std::string, BaseTensor> inputs;
  std::map<std::string, BaseTensor> outputs;
  for (size_t i = 0; i < input_names_.size(); ++i) {
    int64_t input_size =
        std::accumulate(input_shapes_[i].begin(), input_shapes_[i].end(), 1,
                        std::multiplies<int64_t>());
    std::shared_ptr<float> input_data(new float[input_size]);

    // random input
    unsigned int seed = 1;
    for (size_t j = 0; j < input_size; ++j)
      input_data.get()[j] = rand_r(&seed) % 100 / 100.0f;

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
  // warm-up
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

std::string PrecisionBenchmark::GetBenchmarkInfo() const {
  std::stringstream stream;
  stream << Benchmark::GetBenchmarkInfo() << metric_evaluator_type_ << ",";
  return stream.str();
}

Status PrecisionBenchmark::Run() {
  LOG(INFO) << "benchmarking: " << GetBenchmarkInfo();

  Status status = Evaluate();

  if (status != SUCCESS) {
    LOG(WARNING) << "benchmark failed: " << GetBenchmarkInfo();
  } else {
    std::stringstream stream;
    stream << BenchmarkOption::Precision << ":" << GetBenchmarkInfo()
           << post_processor_->GetResult();
    LOG(INFO) << stream.str();
    LogResult(stream.str());
  }
  return status;
}

Status PrecisionBenchmark::Evaluate() {
  Status status;
  // Init the target's environment
  status = executor_->Init(num_threads_);
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }
  // prepare
  status = executor_->Prepare();
  if (status != SUCCESS) {
    executor_->Finish();
    return status;
  }

  std::map<std::string, BaseTensor> inputs;
  std::map<std::string, BaseTensor> outputs;
  for (size_t i = 0; i < input_names_.size(); ++i) {
    int64_t input_size =
        std::accumulate(input_shapes_[i].begin(), input_shapes_[i].end(), 1,
                        std::multiplies<int64_t>());
    auto buffer_in = std::shared_ptr<float>(new float[input_size],
                                            std::default_delete<float[]>());
    inputs[input_names_[i]] = BaseTensor(input_shapes_[i], buffer_in);
  }

  for (size_t i = 0; i < output_names_.size(); ++i) {
    int64_t output_size =
        std::accumulate(output_shapes_[i].begin(), output_shapes_[i].end(), 1,
                        std::multiplies<int64_t>());
    auto buffer_out = std::shared_ptr<float>(new float[output_size],
                                             std::default_delete<float[]>());
    outputs[output_names_[i]] = BaseTensor(output_shapes_[i], buffer_out);
  }

  DIR *dir_parent;
  struct dirent *entry;
  const std::string input_dir("inputs/");
  dir_parent = opendir(input_dir.c_str());
  int count = 0;
  if (dir_parent) {
    while ((entry = readdir(dir_parent))) {
      if (count % 100 == 0) {
        // sleep run_interval seconds to cool off the target
        LOG(INFO) << "sleep " << run_interval_ << "s processed " << count;
        sleep(static_cast<uint32_t>(run_interval_));
      }
      ++count;
      std::string filename = std::string(entry->d_name);

      if (pre_processor_->Run(filename, &inputs) != SUCCESS) continue;

      MACE_CHECK(executor_->Run(inputs, &outputs) == SUCCESS);

      MACE_CHECK(post_processor_->Run(filename, outputs) == SUCCESS);
    }
    closedir(dir_parent);
  }

  executor_->Finish();
  return SUCCESS;
}

}  // namespace benchmark
}  // namespace aibench
