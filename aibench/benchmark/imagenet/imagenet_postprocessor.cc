
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

#include "aibench/benchmark/imagenet/imagenet_postprocessor.h"

#include <algorithm>
#include <functional>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

#include "mace/utils/logging.h"
#include "aibench/benchmark/imagenet/imagenet_preprocessor.h"

namespace aibench {
namespace benchmark {
namespace imagenet {

void GetLabels(const std::string &filename,
               std::vector<std::string> *label_list) {
  std::ifstream in_file(filename, std::ios::in);
  if (in_file.is_open()) {
    std::string label;
    while (std::getline(in_file, label)) {
      label_list->emplace_back(label);
    }
    in_file.close();
  } else {
    LOG(FATAL) << filename << " not found.";
  }
}

int GetFileNum(const std::string &filename) {
  size_t head_len = std::string(imagenet::kImageNameHead).length();
  return atoi(filename.substr(head_len,
                              head_len + imagenet::kImageNameNumLen).c_str());
}

}  // namespace imagenet

ImageNetPostProcessor::ImageNetPostProcessor()
    : total_count_(0),
      correct_count_(0) {
  imagenet::GetLabels("imagenet_groundtruth_labels.txt", &groundtruth_labels_);
  imagenet::GetLabels("mobilenet_model_labels.txt", &model_labels_);
}

Status ImageNetPostProcessor::Run(
    const std::string &filename,
    const std::map<std::string, BaseTensor> &outputs) {
  MACE_CHECK(outputs.size() == 1);
  auto output = outputs.begin();
  int64_t output_size = output->second.size();
  MACE_CHECK(output_size == 1001 || output_size == 1000,
             "Output size should be 1001 or 1000.");
  float *output_data = output->second.data().get();
  auto output_iter = std::max_element(output_data,
                                      output_data + output_size);
  auto output_index = std::distance(output_data, output_iter);
  std::string output_label =
      model_labels_[output_size == 1001 ? output_index : output_index + 1];
  std::string groundtruth_label =
      groundtruth_labels_[imagenet::GetFileNum(filename) - 1];
  if (output_label == groundtruth_label) {
    ++correct_count_;
  }
  ++total_count_;

  return SUCCESS;
}

std::string ImageNetPostProcessor::GetResult() {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(4)
         << 1.0 * correct_count_ / total_count_ << ","
         << total_count_ << "," << correct_count_;
  return stream.str();
}

}  // namespace benchmark
}  // namespace aibench

