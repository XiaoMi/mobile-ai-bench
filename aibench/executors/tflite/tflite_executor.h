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

#ifndef AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_
#define AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "aibench/executors/base_executor.h"
#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/model.h"
#include "tensorflow/contrib/lite/optional_debug_tools.h"
#include "tensorflow/contrib/lite/string.h"
#include "tensorflow/contrib/lite/string_util.h"

namespace aibench {

class TfLiteExecutor : public BaseExecutor {
 public:
  explicit TfLiteExecutor(const std::string &model_file)
      : BaseExecutor(TFLITE, CPU, model_file, "") {}

  virtual Status Init(int num_threads);

  virtual Status Prepare();

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);
  virtual void Finish();
 private:
  std::unique_ptr<tflite::Interpreter> interpreter_;
  std::unique_ptr<tflite::FlatBufferModel> model_;
  int num_threads_;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_TFLITE_TFLITE_EXECUTOR_H_
