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

#include "aibench/executors/tflite/tflite_executor.h"

#include <iostream>

namespace aibench {

Status TfLiteExecutor::Init(const char *model_name, int num_threads) {
  (void)model_name;
  (void)num_threads;
  return Status::SUCCESS;
}

Status TfLiteExecutor::Prepare(const char *model_name) {
  model_ = tflite::FlatBufferModel::BuildFromFile(model_name);
  if (!model_) {
    std::cout << "Failed to mmap model_" << model_name << std::endl;
    return Status::RUNTIME_ERROR;
  }
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model_.get(), resolver);
  builder(&interpreter_);
  if (!interpreter_) {
    std::cout << "Failed to construct interpreter" << std::endl;
    return Status::RUNTIME_ERROR;
  }
  interpreter_->SetNumThreads(4);
  interpreter_->UseNNAPI(false);
  if (interpreter_->AllocateTensors() != kTfLiteOk) {
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
