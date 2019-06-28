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

#include "aibench/executors/tflite/tflite_executor.h"
#include <utility>

#include <iostream>

namespace aibench {
TfLiteExecutor::TfLiteDelegatePtr CreateGPUDelegate(
    tflite::FlatBufferModel* model) {
  TfLiteGpuDelegateOptions options;
  options.metadata = TfLiteGpuDelegateGetModelMetadata(model->GetModel());
  options.compile_options.precision_loss_allowed = 1;
  options.compile_options.preferred_gl_object_type =
      TFLITE_GL_OBJECT_TYPE_FASTEST;
  options.compile_options.dynamic_batch_enabled = 0;
  return TfLiteExecutor::TfLiteDelegatePtr(
      TfLiteGpuDelegateCreate(&options), &TfLiteGpuDelegateDelete);
}

Status TfLiteExecutor::Init(int num_threads) {
  num_threads_ = num_threads;
  return Status::SUCCESS;
}

Status TfLiteExecutor::Prepare() {
  DeviceType device_type = GetDeviceType();
  model_ = tflite::FlatBufferModel::BuildFromFile(GetModelFile().c_str());
  if (!model_) {
    std::cout << "Failed to mmap model_" << GetModelFile() << std::endl;
    return Status::RUNTIME_ERROR;
  }
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model_.get(), resolver);
  builder(&interpreter_);
  if (!interpreter_) {
    std::cout << "Failed to construct interpreter" << std::endl;
    return Status::RUNTIME_ERROR;
  }
  interpreter_->SetNumThreads(num_threads_);
  interpreter_->UseNNAPI(false);
  if (device_type == DeviceType::GPU) {
    TfLiteExecutor::TfLiteDelegatePtr delegate =
        CreateGPUDelegate(model_.get());
    if (!delegate) {
      std::cout << "GPU acceleration is unsupported on this platform."
                   << std::endl;
    } else {
      delegates_.emplace("GPU", std::move(delegate));
    }
    for (const auto& delegate : delegates_) {
      if (interpreter_->ModifyGraphWithDelegate(delegate.second.get()) !=
          kTfLiteOk) {
        std::cout << "Failed to apply" << delegate.first << "delegate.";
      } else {
        std::cout << "Applied" << delegate.first << "delegate.";
      }
    }
  }
  if (delegates_.empty() && interpreter_->AllocateTensors() != kTfLiteOk) {
    std::cout << "Failed to allocate tensors!" << std::endl;
    return Status::RUNTIME_ERROR;
  }

  return Status::SUCCESS;
}

Status TfLiteExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                           std::map<std::string, BaseTensor> *outputs) {
  (void)inputs;
  (void)outputs;
  TfLiteStatus run_status = interpreter_->Invoke();
  return run_status == kTfLiteOk ? Status::SUCCESS
                                 : Status::RUNTIME_ERROR;
}

void TfLiteExecutor::Finish() {
  interpreter_.reset();
}

}  // namespace aibench
