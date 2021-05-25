// Copyright 2021 The MobileAIBench Authors. All Rights Reserved.
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


#include "aibench/executors/tnn/tnn_executor.h"

int tnn_num_threads;

namespace aibench {

Status TnnExecutor::Prepare() {
  const DeviceType device_type = GetDeviceType();
  TNN_NS::ModelConfig model_config;
  std::ifstream model_graph_file(GetModelFile().c_str());
  std::string model_graph_data;
  model_graph_data = std::string((std::istreambuf_iterator<char>
                                  (model_graph_file)),
                                 std::istreambuf_iterator<char>());

  model_config.params.push_back(model_graph_data);

  std::ifstream model_weights_file(GetWeightFile().c_str());
  std::string model_weights_data;
  model_weights_data = std::string((std::istreambuf_iterator<char>
                                    (model_weights_file)),
                                   std::istreambuf_iterator<char>());

  model_config.params.push_back(model_weights_data);

  tnn_.Init(model_config);

  TNN_NS::Status error;

  TNN_NS::NetworkConfig cpu_network_config;
  if (device_type == CPU) {
    cpu_network_config.device_type = TNN_NS::DEVICE_ARM;
  } else if (device_type == GPU) {
    cpu_network_config.device_type = TNN_NS::DEVICE_OPENCL;
  }
  instance_ = tnn_.CreateInst(cpu_network_config, error);
  instance_->SetCpuNumThreads(tnn_num_threads);
  return error == TNN_NS::TNN_OK ? Status::SUCCESS : Status::RUNTIME_ERROR;
}

Status TnnExecutor::Init(int num_threads) {
  tnn_num_threads = num_threads;
  LOG(INFO) << "TnnExecutor num_threads: " << num_threads;
  return Status::SUCCESS;
}

Status TnnExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs) {
  if (!instance_) {
    return {};
  }

  void* command_queue;
  instance_->GetCommandQueue(reinterpret_cast<void**>(&command_queue));

  TNN_NS::BlobMap input_blobs;
  instance_->GetAllInputBlobs(input_blobs);
  // check inputs
  auto input_name = inputs.find(input_blobs.begin()->first);
  if (input_name == inputs.end()) {
    LOG(ERROR) << "input name not matched";
    return Status::RUNTIME_ERROR;
  }
  void* data = inputs.at(input_blobs.begin()->first).data().get();
  std::vector<int64_t> shape = inputs.at(input_blobs.begin()->first).shape();
  std::vector<int> tnn_shape(shape.begin(), shape.end());
  TNN_NS::Blob* input = input_blobs.begin()->second;
  TNN_NS::Mat input_mat(TNN_NS::DEVICE_ARM, TNN_NS::NCHW_FLOAT,
                        tnn_shape, data);

  TNN_NS::BlobConverter input_blob_convert(input);
  TNN_NS::MatConvertParam input_convert_param;
  input_blob_convert.ConvertFromMat(input_mat, input_convert_param,
                                    command_queue);

  instance_->Forward();

  TNN_NS::BlobMap output_blobs;
  instance_->GetAllOutputBlobs(output_blobs);
  TNN_NS::Blob* output = output_blobs.begin()->second;

  // check outputs
  auto output_name = outputs->find(output_blobs.begin()->first);
  if (output_name == outputs->end()) {
    LOG(ERROR) << "output name not matched";
    return Status::RUNTIME_ERROR;
  }

  return Status::SUCCESS;
}

void TnnExecutor::Finish() {
  if (!instance_) {
    return;
  }
  instance_->DeInit();
}

}  // namespace aibench
