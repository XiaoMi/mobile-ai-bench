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

#include "aibench/executors/hiai/hiai_executor.h"

#include <iostream>
#include <utility>
#include <vector>

#include "aibench/utils/logging.h"

namespace aibench {

namespace {
  const int kHiAiOk = 0;
}

Status HiAiExecutor::Init(int num_threads) {
  return Status::SUCCESS;
}

Status HiAiExecutor::Prepare() {
  mix_model_manager_ = MixModelManagerUniquePtr(
      HIAI_MixModelManager_Create(nullptr),
      HIAI_MixModelManager_Destroy);
  MACE_CHECK(mix_model_manager_ != nullptr,
             "Fail to create mix mode manager.");

  std::string model_path = GetModelFile();
  bool compatible = HIAI_CheckMixModelCompatibility_From_File(
      mix_model_manager_.get(),
      false,
      model_path.c_str());
  if (!compatible) {
    std::string ddk_version =
        HIAI_ModelManager_GetVersion(mix_model_manager_.get());
    LOG(FATAL) << "Model is not compatible! modle path: " << model_path
               << ", DDK version: " << ddk_version;
  }

  HIAI_MixModelBuffer *mix_model_buffer =
      HIAI_MixModelBuffer_Create_From_File(model_name_.c_str(),
                                           model_path.c_str(),
                                           HIAI_MIX_DEVPREF_HIGH,
                                           false);
  mix_model_buffer_ = MixModelBufferUniquePtr(mix_model_buffer,
                                              HIAI_MixModelBuffer_Destroy);
  MACE_CHECK(mix_model_buffer_ != nullptr,
             "Fail to create model buffer from file:",
             model_path);

  HIAI_MixModelBuffer *model_buffer_array[] = {mix_model_buffer_.get()};
  int ret = HIAI_MixModel_LoadFromModelBuffers(mix_model_manager_.get(),
                                               model_buffer_array, 1);
  MACE_CHECK(ret == kHiAiOk, "Fail to load model.");

  return Status::SUCCESS;
}

Status HiAiExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs) {
  if (input_tensors_.size() == 0 &&
      Status::SUCCESS != CreateHiAiTensorFromBaseTensor(&inputs,
                                                        &input_tensors_)) {
    LOG(FATAL) << "Fail to create hiai input tensor.";
  }

  if (Status::SUCCESS != CopyDataBetweenHiAiAndBaseTensors(
        const_cast<std::map<std::string, BaseTensor> *>(&inputs),
        &input_tensors_, true)) {
    LOG(FATAL) << "Fail to copy data from base tensor to hiai.";
  }

  if (output_tensors_.size() == 0 &&
      Status::SUCCESS != CreateHiAiTensorFromBaseTensor(outputs,
                                                        &output_tensors_)) {
    LOG(FATAL) << "Fail to create hiai output tensor.";
  }

  int run_model_ret = HIAI_MixModel_RunModel(mix_model_manager_.get(),
                                             input_tensors_.data(),
                                             input_tensors_.size(),
                                             output_tensors_.data(),
                                             output_tensors_.size(),
                                             10000,  // timeout, in milliseconds
                                             model_name_.c_str());
  MACE_CHECK(run_model_ret == kHiAiOk, "Fail to run model, ret=",
             run_model_ret);

  if (Status::SUCCESS != CopyDataBetweenHiAiAndBaseTensors(outputs,
                                                           &output_tensors_,
                                                           false)) {
    LOG(FATAL) << "Fail to copy data from hiai to base tensor.";
  }

  return Status::SUCCESS;
}

void HiAiExecutor::Finish() {
  DestroyHiAiTensor(&output_tensors_);
  DestroyHiAiTensor(&input_tensors_);
  mix_model_buffer_.reset();
  mix_model_manager_.reset();
}

Status HiAiExecutor::CreateHiAiTensorFromBaseTensor(
    const std::map<std::string, BaseTensor> *base_tensor_map,
    std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec) {
  MACE_CHECK(tensor_buffer_vec->size() == 0, "Invalid tensor size.");

  for (const auto &base_tensor_item : *base_tensor_map) {
    const std::vector<int64_t> &shape = base_tensor_item.second.shape();
    int shape_size = shape.size();
    MACE_CHECK(shape_size != 0, "Get an invalid BaseTensor, shape size is 0.");

    // NCHW
    int dim_n = shape[0];
    int dim_c = shape_size >= 2 ? shape[1] : 1;
    int dim_h = shape_size >= 3 ? shape[2] : 1;
    int dim_w = shape_size >= 4 ? shape[3] : 1;
    HIAI_MixTensorBuffer *tensor_buffer =
        HIAI_MixTensorBuffer_Create(dim_n, dim_c, dim_h, dim_w);
    MACE_CHECK(tensor_buffer != nullptr, "Faile to create hiai tensor buffer.");

    tensor_buffer_vec->push_back(tensor_buffer);
  }

  if (create_status != Status::SUCCESS) {
    DestroyHiAiTensor(tensor_buffer_vec);
  }

  return create_status;
}

Status HiAiExecutor::CopyDataBetweenHiAiAndBaseTensors(
    std::map<std::string, BaseTensor> *base_tensor_map,
    std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec,
    bool base_to_hiai) {

  MACE_CHECK(base_tensor_map->size() == tensor_buffer_vec->size(),
             "Faile to copy data between hiai and base tensors.");

  int pos = 0;
  for (auto &base_tensor_item : *base_tensor_map) {
    BaseTensor* base_tensor = &(base_tensor_item.second);
    HIAI_MixTensorBuffer *mix_tensor_buffer = (*tensor_buffer_vec)[pos];
    void *raw_buffer = HIAI_MixTensorBuffer_GetRawBuffer(mix_tensor_buffer);
    int buffer_size = HIAI_MixTensorBuffer_GetBufferSize(mix_tensor_buffer);
    if (base_to_hiai) {
      memcpy(raw_buffer, base_tensor->data().get(), buffer_size);
    } else {
      memcpy(base_tensor->data().get(), raw_buffer, buffer_size);
    }
    ++pos;
  }

  return Status::SUCCESS;
}

void HiAiExecutor::DestroyHiAiTensor(
    std::vector<HIAI_MixTensorBuffer *> *tensor_buffer_vec) {
  for (size_t i = 0; i < tensor_buffer_vec->size(); ++i) {
    HIAI_MixTensorBuffer *tensor_buffer = (*tensor_buffer_vec)[i];
    if (tensor_buffer != nullptr) {
      HIAI_MixTensorBufferr_Destroy(tensor_buffer);
      (*tensor_buffer_vec)[i] = nullptr;
    }
  }
  tensor_buffer_vec->clear();
}


}  // namespace aibench
