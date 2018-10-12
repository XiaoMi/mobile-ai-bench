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

#include "aibench/executors/ncnn/ncnn_executor.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "ncnn/include/cpu.h"

namespace ncnn {

int BenchNet::load_model() {
  ModelBinFromEmpty mb;
  for (size_t i = 0; i < layers.size(); ++i) {
    Layer *layer = layers[i];
    int ret = layer->load_model(mb);
    if (ret != 0) {
      fprintf(stderr, "layer load_model %d failed\n", static_cast<int>(i));
      return -1;
    }
  }

  return 0;
}

}  // namespace ncnn

namespace aibench {

Status NcnnExecutor::Init(const char *model_name, int num_threads) {
  static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
  static ncnn::PoolAllocator g_workspace_pool_allocator;

  (void) model_name;
  ncnn::set_cpu_powersave(0);
  ncnn::set_omp_dynamic(0);
  ncnn::set_omp_num_threads(num_threads);

  g_blob_pool_allocator.clear();
  g_workspace_pool_allocator.clear();

  ncnn::Option opt;
  opt.lightmode = true;
  opt.num_threads = num_threads;
  opt.blob_allocator = &g_blob_pool_allocator;
  opt.workspace_allocator = &g_workspace_pool_allocator;
  ncnn::set_default_option(opt);
  return Status::SUCCESS;
}

Status NcnnExecutor::Prepare(const char *model_name) {
  int ret;

  ret = net.load_param(model_name);
  if (ret != 0) {
    return Status::NOT_SUPPORTED;
  }

  ret = net.load_model();
  if (ret != 0) {
    return Status::RUNTIME_ERROR;
  }

  return Status::SUCCESS;
}

Status NcnnExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs) {
  (void) outputs;
  // check inputs and outputs
  auto input = inputs.find("data");
  if (input == inputs.end()) {
    return Status::RUNTIME_ERROR;
  }
  // transform inputs
  const std::vector<int64_t> &shape = input->second.shape();
  const std::shared_ptr<float> data = input->second.data();
  ncnn::Mat in(static_cast<int>(shape[0]),
               static_cast<int>(shape[1]),
               static_cast<int>(shape[2]),
               data.get());

  // Execute the network inference and retrieve the result
  ncnn::Extractor ex = net.create_extractor();
  ex.input("data", in);
  ncnn::Mat out;
  ex.extract("prob", out);

  return Status::SUCCESS;
}

void NcnnExecutor::Finish() {
  net.clear();
}

}  // namespace aibench
