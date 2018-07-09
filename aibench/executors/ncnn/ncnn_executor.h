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

#ifndef AIBENCH_EXECUTORS_NCNN_NCNN_EXECUTOR_H_
#define AIBENCH_EXECUTORS_NCNN_NCNN_EXECUTOR_H_

#include <map>
#include <string>

#include "aibench/executors/base_executor.h"
#include "ncnn/include/layer.h"
#include "ncnn/include/mat.h"
#include "ncnn/include/modelbin.h"
#include "ncnn/include/net.h"

namespace ncnn {

// always return empty weights
class ModelBinFromEmpty : public ModelBin {
 public:
  virtual Mat load(int w, int /*type*/) const { return Mat(w); }
};

class BenchNet : public Net {
 public:
  int load_model();
};

}  // namespace ncnn

namespace aibench {

class NcnnExecutor : public BaseExecutor {
 public:
  NcnnExecutor() : BaseExecutor(NCNN, CPU) {}

  virtual Status Init(const char *model_name, int num_threads);

  virtual Status Prepare(const char *model_name);

  virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                     std::map<std::string, BaseTensor> *outputs);

  virtual void Finish();
 private:
  ncnn::BenchNet net;
};

}  // namespace aibench

#endif  // AIBENCH_EXECUTORS_NCNN_NCNN_EXECUTOR_H_
