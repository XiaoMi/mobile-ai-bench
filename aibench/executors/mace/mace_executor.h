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

#ifndef AIBENCH_EXECUTORS_MACE_MACE_EXECUTOR_H_
#define AIBENCH_EXECUTORS_MACE_MACE_EXECUTOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "aibench/executors/base_executor.h"
#include "mace/public/mace.h"

namespace aibench {

class MaceExecutor : public BaseExecutor {
 public:
  MaceExecutor(DeviceType device_type,
               const std::string &model_file,
               const std::string &weight_file,
               const std::vector<std::string> &input_names,
               const std::vector<std::string> &output_names)
      : BaseExecutor(MACE, device_type, model_file, weight_file),
        input_names_(input_names),
        output_names_(output_names) {}

  virtual Status Init(int num_threads);

  virtual Status Prepare();

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);
  virtual void Finish();

  Status CreateEngine(std::shared_ptr<mace::MaceEngine> *engine);

 private:
  std::vector<std::string> input_names_;
  std::vector<std::string> output_names_;
  std::shared_ptr<mace::MaceEngine> engine_;
  int num_threads_;
  std::shared_ptr<mace::GPUContext> gpu_context_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_MACE_MACE_EXECUTOR_H_
