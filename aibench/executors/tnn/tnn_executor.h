// Copyright 2021 The MobileAIBench Authors. All Rights Reserved.
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

#ifndef AIBENCH_EXECUTORS_TNN_TNN_EXECUTOR_H_
#define AIBENCH_EXECUTORS_TNN_TNN_EXECUTOR_H_

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "aibench/executors/base_executor.h"
#include "include/tnn/core/tnn.h"
#include "include/tnn/core/instance.h"
#include "include/tnn/core/common.h"
#include "include/tnn/utils/dims_vector_utils.h"
#include "include/tnn/utils/blob_converter.h"

namespace aibench {

class TnnExecutor : public BaseExecutor {
 public:
  explicit TnnExecutor(DeviceType device_type,
               const std::string &model_file,
               const std::string &weight_file)
      : BaseExecutor(TNN, device_type, model_file, weight_file) {}

  Status Init(int num_threads) override;

  Status Prepare() override;

  Status Run(const std::map<std::string, BaseTensor> &inputs,
             std::map<std::string, BaseTensor> *outputs) override;

  void Finish() override;
 private:
  TNN_NS::TNN tnn_;
  std::shared_ptr<TNN_NS::Instance> instance_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_TNN_TNN_EXECUTOR_H_
