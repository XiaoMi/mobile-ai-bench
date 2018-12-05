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

#ifndef AIBENCH_BENCHMARK_BENCHMARK_H_
#define AIBENCH_BENCHMARK_BENCHMARK_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "aibench/executors/base_executor.h"

namespace aibench {
namespace benchmark {

class Benchmark {
 public:
  Benchmark(BaseExecutor *executor,
            const ModelName &model_name,
            const bool quantize,
            const std::vector<std::string> &input_names,
            const std::vector<std::vector<int64_t>> &input_shapes,
            const std::vector<std::string> &output_names,
            const std::vector<std::vector<int64_t>> &output_shapes,
            const int run_interval,
            const int num_threads);

  virtual Status Run() = 0;
  virtual ~Benchmark() = default;

 protected:
  virtual std::string GetBenchmarkInfo() const;
  Status LogResult(const std::string &result);

  BaseExecutor *executor_;
  ModelName model_name_;
  bool quantize_;
  std::vector<std::string> input_names_;
  std::vector<std::vector<int64_t>> input_shapes_;
  std::vector<std::string> output_names_;
  std::vector<std::vector<int64_t>> output_shapes_;
  int run_interval_;
  int num_threads_;
};

class PerformanceBenchmark : public Benchmark {
 public:
  PerformanceBenchmark(BaseExecutor *executor,
                       const ModelName &model_name,
                       const bool quantize,
                       const std::vector<std::string> &input_names,
                       const std::vector<std::vector<int64_t>> &input_shapes,
                       const std::vector<std::string> &output_names,
                       const std::vector<std::vector<int64_t>> &output_shapes,
                       const int run_interval,
                       const int num_threads)
      : Benchmark(executor,
                  model_name,
                  quantize,
                  input_names,
                  input_shapes,
                  output_names,
                  output_shapes,
                  run_interval,
                  num_threads) {}

 public:
  Status Run() override;

 private:
  Status Run(double *init_seconds, double *run_seconds);
};

class PreProcessor {
 public:
  virtual Status Run(const std::string &filename,
                     std::map<std::string, BaseTensor> *inputs) = 0;
};
class PostProcessor {
 public:
  virtual Status Run(const std::string &filename,
                     const std::map<std::string, BaseTensor> &outputs) = 0;
  virtual std::string GetResult() = 0;
};

class PrecisionBenchmark : public Benchmark {
 public:
  PrecisionBenchmark(
      BaseExecutor *executor,
      const ModelName &model_name,
      const bool quantize,
      const std::vector<std::string> &input_names,
      const std::vector<std::vector<int64_t>> &input_shapes,
      const std::vector<std::string> &output_names,
      const std::vector<std::vector<int64_t>> &output_shapes,
      const int run_interval,
      const int num_threads,
      std::unique_ptr<PreProcessor> pre_processor,
      std::unique_ptr<PostProcessor> post_processor,
      const MetricEvaluator_MetricEvaluatorType metric_evaluator_type)
      : Benchmark(executor,
                  model_name,
                  quantize,
                  input_names,
                  input_shapes,
                  output_names,
                  output_shapes,
                  run_interval,
                  num_threads),
        pre_processor_(std::move(pre_processor)),
        post_processor_(std::move(post_processor)),
        metric_evaluator_type_(metric_evaluator_type) {}

  Status Run() override;

 protected:
  std::string GetBenchmarkInfo() const override;

 private:
  Status Evaluate();

  std::unique_ptr<PreProcessor> pre_processor_;
  std::unique_ptr<PostProcessor> post_processor_;
  MetricEvaluator_MetricEvaluatorType metric_evaluator_type_;
};

int64_t NowMicros();

}  // namespace benchmark
}  // namespace aibench

#endif  // AIBENCH_BENCHMARK_BENCHMARK_H_
