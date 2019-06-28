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

#ifndef AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_
#define AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "aibench/executors/base_executor.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/string.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/delegates/gpu/gl_delegate.h"

namespace aibench {

class TfLiteExecutor : public BaseExecutor {
 public:
  using TfLiteDelegatePtr = tflite::Interpreter::TfLiteDelegatePtr;
  using TfLiteDelegatePtrMap = std::map<std::string, TfLiteDelegatePtr>;
  explicit TfLiteExecutor(DeviceType device_type,
                          const std::string &model_file)
      : BaseExecutor(TFLITE, device_type, model_file, "") {}

  virtual Status Init(int num_threads);

  virtual Status Prepare();

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);
  virtual void Finish();
 private:
  int num_threads_;
  std::unique_ptr<tflite::Interpreter> interpreter_;
  std::unique_ptr<tflite::FlatBufferModel> model_;
  TfLiteDelegatePtrMap delegates_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_
