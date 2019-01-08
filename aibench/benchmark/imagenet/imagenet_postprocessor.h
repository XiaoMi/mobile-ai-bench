// Copyright 2018 The MobileAIBench Authors. All Rights Reserved.
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

#ifndef AIBENCH_BENCHMARK_IMAGENET_IMAGENET_POSTPROCESSOR_H_
#define AIBENCH_BENCHMARK_IMAGENET_IMAGENET_POSTPROCESSOR_H_

#include <map>
#include <string>
#include <vector>

#include "aibench/benchmark/benchmark.h"

namespace aibench {
namespace benchmark {

class ImageNetPostProcessor : public PostProcessor {
 public:
  ImageNetPostProcessor();
  Status Run(const std::string &filename,
             const std::map<std::string, BaseTensor> &outputs) override;
  std::string GetResult() override;

 private:
  std::vector<std::string> groundtruth_labels_;
  std::vector<std::string> model_labels_;
  int total_count_;
  int correct_count_;
};

}  // namespace benchmark
}  // namespace aibench

#endif  // AIBENCH_BENCHMARK_IMAGENET_IMAGENET_POSTPROCESSOR_H_
