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

#include "nnbench/executors/mace/mace_executor.h"

#include <sys/types.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace nnbench {

inline Status ReadBinaryFile(std::vector<unsigned char> *data,
                           const std::string &filename) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    return Status::RUNTIME_ERROR;
  }
  ifs.seekg(0, ifs.end);
  size_t length = ifs.tellg();
  ifs.seekg(0, ifs.beg);

  data->reserve(length);
  data->insert(data->begin(), std::istreambuf_iterator<char>(ifs),
               std::istreambuf_iterator<char>());
  if (ifs.fail()) {
    return Status::RUNTIME_ERROR;
  }
  ifs.close();

  return Status::SUCCESS;
}

std::string GetOpenclBinaryPath(const std::string &model_name,
                                const std::string &product_soc) {
  std::vector<std::string> files;
  DIR *dir = opendir(".");
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    files.push_back(entry->d_name);
  }
  closedir(dir);
  for (const auto &file : files) {
    // e.g. mobilenet_v1_compiled_opencl_kernel.MIX2S.sdm845.bin
    if (file.find(model_name) != std::string::npos
        && file.find(product_soc) != std::string::npos) {
      return file;
    }
  }

  return "";
}

mace::DeviceType GetDeviceType(const Runtime &runtime) {
  switch (runtime) {
    case CPU: return mace::CPU;
    case GPU: return mace::GPU;
    case DSP: return mace::HEXAGON;
    default: return mace::CPU;
  }
}

Status MaceExecutor::Prepare(const char *model_name) {
  mace::DeviceType device_type = GetDeviceType(GetRuntime());
  mace::SetOpenMPThreadPolicy(4, static_cast<mace::CPUAffinityPolicy>(1));
  if (device_type == mace::GPU) {
    mace::SetGPUHints(
        static_cast<mace::GPUPerfHint>(3),
        static_cast<mace::GPUPriorityHint>(3));
    std::string opencl_binary_path =
        GetOpenclBinaryPath(model_name, product_soc_);
    if (opencl_binary_path == "") {
      std::cout << model_name << " opencl binary file not found." << std::endl;
      return RUNTIME_ERROR;
    }
    std::vector<std::string> opencl_binary_paths = {opencl_binary_path};
    mace::SetOpenCLBinaryPaths(opencl_binary_paths);

    const std::string kernel_file_path("/data/local/tmp/mace_run/interior");
    std::shared_ptr<mace::KVStorageFactory> storage_factory(
        new mace::FileStorageFactory(kernel_file_path));
    mace::SetKVStorageFactory(storage_factory);
  }

  std::vector<unsigned char> model_pb_data;
  std::string model_pb_file(model_name);
  model_pb_file.append(".pb");
  if (ReadBinaryFile(&model_pb_data, model_pb_file) != Status::SUCCESS) {
    std::cout << "Failed to read file: " << model_name << std::endl;
    return Status::RUNTIME_ERROR;
  }
  std::string model_data_file(model_name);
  model_data_file.append(".data");

  mace::MaceStatus create_engine_status;
  create_engine_status =
      CreateMaceEngineFromProto(model_pb_data,
                                model_data_file,
                                input_names_,
                                output_names_,
                                device_type,
                                &engine_);

  return create_engine_status == mace::MACE_SUCCESS ? Status::SUCCESS
                                                    : Status::RUNTIME_ERROR;
}

Status MaceExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs) {
  (void)outputs;

  std::map<std::string, mace::MaceTensor> mace_inputs;
  std::map<std::string, mace::MaceTensor> mace_outputs;
  for (const auto &input : inputs) {
    mace_inputs[input.first] = mace::MaceTensor(input.second.shape(),
                                                input.second.data());
  }
  for (const auto &output : *outputs) {
    mace_outputs[output.first] = mace::MaceTensor(output.second.shape(),
                                                  output.second.data());
  }
  mace::MaceStatus run_status = engine_->Run(mace_inputs, &mace_outputs);

  return run_status == mace::MACE_SUCCESS ? Status::SUCCESS
                                          : Status::RUNTIME_ERROR;
}

void MaceExecutor::Finish() {
  engine_.reset();
}

}  // namespace nnbench
