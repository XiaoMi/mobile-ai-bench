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

#include "aibench/executors/mnn/mnn_executor.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace aibench {

Status MnnExecutor::Init(int num_threads) {
  config_.numThread = num_threads;
  DeviceType device_type = GetDeviceType();
  if (device_type == DeviceType::CPU) {
    config_.type = MNN_FORWARD_CPU;
  } else if (device_type == DeviceType::GPU) {
    config_.type = MNN_FORWARD_OPENCL;
  } else if (device_type == DeviceType::VULKAN) {
    config_.type = MNN_FORWARD_VULKAN;
  } else {
    return Status::NOT_SUPPORTED;
  }
  return Status::SUCCESS;
}

Status MnnExecutor::Prepare() {
  net_ = std::shared_ptr<MNN::Interpreter>(
      MNN::Interpreter::createFromFile(GetModelFile().c_str()));
  session_ = net_->createSession(config_);
  net_->releaseModel();
  return Status::SUCCESS;
}

Status MnnExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs) {
  (void) inputs;
  (void) outputs;
  auto input = net_->getSessionInput(session_, NULL);
  const MNN::Backend* inBackend = net_->getBackend(session_, input);
  std::shared_ptr<MNN::Tensor> givenTensor(
      MNN::Tensor::createHostTensorFromDevice(input, false));
  auto outputTensor = net_->getSessionOutput(session_, NULL);
  std::shared_ptr<MNN::Tensor> expectTensor(
      MNN::Tensor::createHostTensorFromDevice(outputTensor, false));
  input->copyFromHostTensor(givenTensor.get());
  net_->runSession(session_);
  outputTensor->copyToHostTensor(expectTensor.get());
  return Status::SUCCESS;
}

void MnnExecutor::Finish() {
}

}  // namespace aibench
