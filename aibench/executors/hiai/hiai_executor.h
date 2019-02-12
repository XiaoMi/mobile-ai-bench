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

#ifndef AIBENCH_EXECUTORS_HIAI_HIAI_EXECUTOR_H_
#define AIBENCH_EXECUTORS_HIAI_HIAI_EXECUTOR_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "aibench/executors/base_executor.h"
#include "HIAIMixModel.h"

namespace aibench {

class HiAiExecutor : public BaseExecutor {
 public:
  explicit HiAiExecutor(const std::string &model_file,
                        const std::string &model_name)
      : BaseExecutor(HIAI, NPU, model_file, ""),
        model_name_(model_name),
        mix_model_manager_(nullptr, HIAI_MixModelManager_Destroy),
        mix_model_buffer_(nullptr, HIAI_MixModelBuffer_Destroy) {}

  virtual Status Init(int num_threads);

  virtual Status Prepare();

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);
  virtual void Finish();

 private:
  Status CreateHiAiTensorFromBaseTensor(
      const std::map<std::string, BaseTensor> *base_tensor_map,
      std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec);

  Status CopyDataBetweenHiAiAndBaseTensors(
      std::map<std::string, BaseTensor> *base_tensor_map,
      std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec,
      bool base_to_hiai);

  void DestroyHiAiTensor(
      std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec);

  typedef std::unique_ptr<
      HIAI_MixModelManager, decltype(HIAI_MixModelManager_Destroy) *>
      MixModelManagerUniquePtr;

  typedef std::unique_ptr<
      HIAI_MixModelBuffer, decltype(HIAI_MixModelBuffer_Destroy) *>
      MixModelBufferUniquePtr;

  MixModelManagerUniquePtr mix_model_manager_;
  MixModelBufferUniquePtr mix_model_buffer_;
  std::vector<HIAI_MixTensorBuffer *> input_tensors_;
  std::vector<HIAI_MixTensorBuffer *> output_tensors_;

  std::string model_name_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_HIAI_HIAI_EXECUTOR_H_
