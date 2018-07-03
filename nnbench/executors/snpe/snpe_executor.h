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

#ifndef NNBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_
#define NNBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_

#include <map>
#include <memory>
#include <string>

#include "nnbench/executors/base_executor.h"
#include "SNPE/SNPE.hpp"

namespace nnbench {

class SnpeExecutor : public BaseExecutor {
 public:
  explicit SnpeExecutor(Runtime runtime) : BaseExecutor(SNPE, runtime) {}

  virtual Status Init(const char *model_name, int num_threads);

  virtual Status Prepare(const char *model_name);

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);

  virtual void Finish();
 private:
  std::unique_ptr<zdl::SNPE::SNPE> snpe_;
  zdl::DlSystem::TensorMap input_tensor_map_;
  zdl::DlSystem::TensorMap output_tensor_map_;
};

}  // namespace nnbench

#endif  // NNBENCH_EXECUTORS_SNPE_SNPE_EXECUTOR_H_
