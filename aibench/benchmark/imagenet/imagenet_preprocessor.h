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

#ifndef AIBENCH_BENCHMARK_IMAGENET_IMAGENET_PREPROCESSOR_H_
#define AIBENCH_BENCHMARK_IMAGENET_IMAGENET_PREPROCESSOR_H_

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "aibench/benchmark/benchmark.h"

namespace aibench {
namespace benchmark {
namespace imagenet {

extern const char *kImageNameHead;
extern const char *kImageNameTail;
extern const int kImageNameNumLen;

}  // namespace imagenet

class ImageNetPreProcessor : public PreProcessor {
 public:
  ImageNetPreProcessor(const std::vector<DataFormat> &data_formats,
                       const std::vector<std::vector<float>> &input_means,
                       const std::vector<float> &input_var,
                       const ChannelOrder channel_order);
  Status Run(const std::string &filename,
             std::map<std::string, BaseTensor> *inputs) override;

 private:
  std::vector<DataFormat> data_formats_;
  std::vector<std::vector<float>> input_means_;
  std::vector<float> input_var_;
  ChannelOrder channel_order_;
  std::unordered_set<std::string> blacklist_;
};

}  // namespace benchmark
}  // namespace aibench

#endif  // AIBENCH_BENCHMARK_IMAGENET_IMAGENET_PREPROCESSOR_H_
