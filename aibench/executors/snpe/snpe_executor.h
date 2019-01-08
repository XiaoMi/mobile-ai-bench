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

#ifndef AIBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_
#define AIBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_

#include <map>
#include <memory>
#include <string>

#include "aibench/executors/base_executor.h"
#include "SNPE/SNPE.hpp"

namespace aibench {

class SnpeExecutor : public BaseExecutor {
 public:
  explicit SnpeExecutor(DeviceType device_type,
                        const std::string &model_file)
      : BaseExecutor(SNPE, device_type, model_file, "") {}

  virtual Status Init(int num_threads);

  virtual Status Prepare();

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);

  virtual void Finish();
 private:
  std::unique_ptr<zdl::SNPE::SNPE> snpe_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_
