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

#include "nnbench/executors/snpe/snpe_executor.h"

#include <map>
#include <string>
#include <iostream>
#include <cstring>
#include <random>
#include <algorithm>
#include <vector>

#include "DiagLog/IDiagLog.hpp"
#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/ITensorFactory.hpp"
#include "DlSystem/UDLFunc.hpp"
#include "DlSystem/IUserBuffer.hpp"
#include "SNPE/SNPE.hpp"
#include "SNPE/SNPEBuilder.hpp"
#include "SNPE/SNPEFactory.hpp"

namespace nnbench {

namespace {

std::unique_ptr<zdl::DlContainer::IDlContainer> LoadContainerFromFile(
    const char *containerPath) {
  std::unique_ptr<zdl::DlContainer::IDlContainer> container;
  container = zdl::DlContainer::IDlContainer::open(
      zdl::DlSystem::String(containerPath));
  return container;
}

std::unique_ptr<zdl::SNPE::SNPE> SetBuilderOptions(
    const std::unique_ptr<zdl::DlContainer::IDlContainer> &container,
    zdl::DlSystem::Runtime_t runtime,
    zdl::DlSystem::UDLBundle udlBundle,
    bool useUserSuppliedBuffers) {
  std::unique_ptr<zdl::SNPE::SNPE> snpe;
  zdl::SNPE::SNPEBuilder snpeBuilder(container.get());
  snpe = snpeBuilder.setOutputLayers({})
      .setRuntimeProcessor(runtime)
      .setUdlBundle(udlBundle)
      .setUseUserSuppliedBuffers(useUserSuppliedBuffers)
      .build();

  return snpe;
}

std::unique_ptr<zdl::SNPE::SNPE> BuildSnpeRuntime(
    const char *model_name, zdl::DlSystem::Runtime_t runtime) {
  std::cout << "building snpe with model_name: " << model_name << " runtime:"
            << static_cast<int>(runtime) << std::endl;
  zdl::DlSystem::UDLBundle udlBundle;
  // 0xdeadbeaf to test cookie
  udlBundle.cookie = reinterpret_cast<void *>(0xdeadbeaf);

  if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime)) {
    std::cerr << "SNPE Runtime " << static_cast<int>(runtime)
              << " not available! " << std::endl;
    return nullptr;
  }

  std::unique_ptr<zdl::DlContainer::IDlContainer>
      container = LoadContainerFromFile(model_name);
  auto snpe = SetBuilderOptions(container, runtime, udlBundle, false);
  std::cout << "snpe build ok" << std::endl;
  return snpe;
}

Status ProcessInput(zdl::SNPE::SNPE *snpe,
                    const std::map<std::string, BaseTensor> &inputs,
                    zdl::DlSystem::TensorMap *input_tensor_map) {
  const auto &input_tensor_names_ref = snpe->getInputTensorNames();
  const auto &input_tensor_names = *input_tensor_names_ref;
  if (inputs.size() != input_tensor_names.size()) {
    std::cerr << "inputs size not matched" << std::endl;
    return Status::RUNTIME_ERROR;
  }
  std::unique_ptr<zdl::DlSystem::ITensor> input_tensor;
  for (size_t i = 0; i < input_tensor_names.size(); i++) {
    std::string input_name(input_tensor_names.at(i));
    const auto &input_shape_opt =
        snpe->getInputDimensions(input_tensor_names.at(i));
    const auto &input_shape = *input_shape_opt;
    input_tensor =
        zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(input_shape);
    size_t input_size = inputs.at(input_name).size();
    std::vector<float> input_vec(inputs.at(input_name).data().get(),
                                 inputs.at(input_name).data().get()
                                     + input_size);

    std::copy(input_vec.begin(), input_vec.end(), input_tensor.get()->begin());
    input_tensor_map->add(input_name.c_str(), input_tensor.release());
  }
  return Status::SUCCESS;
}

Status ProcessOutput(const zdl::DlSystem::TensorMap &output_tensor_map,
                     std::map<std::string, BaseTensor> *outputs) {
  auto tensor_names = output_tensor_map.getTensorNames();
  for (size_t i = 0; i < tensor_names.size(); ++i) {
    std::string output_name(tensor_names.at(i));
    zdl::DlSystem::ITensor* output_tensor =
        output_tensor_map.getTensor(output_name.c_str());
    std::shared_ptr<float> out_data(new float[output_tensor->getSize()]);
    std::copy(output_tensor->begin(), output_tensor->end(),
              out_data.get());
    auto output_shape = output_tensor->getShape();
    std::vector<int64_t> out_shape(output_shape.rank());
    for (size_t j = 0; j < out_shape.size(); ++j) {
      out_shape[j] = output_shape[j];
    }
    outputs->insert({output_name, BaseTensor(out_shape, out_data)});
  }
  return Status::SUCCESS;
}

}  // namespace

Status SnpeCPUExecutor::Prepare(const char *model_name) {
  static zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::Runtime_t::CPU;
  snpe_ = BuildSnpeRuntime(model_name, runtime);
  if (snpe_ == nullptr) return Status::RUNTIME_ERROR;
  return Status::SUCCESS;
}

Status SnpeCPUExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  Status status;
  // step1: prepare inputs
  input_tensor_map_.clear();
  status = ProcessInput(snpe_.get(), inputs, &input_tensor_map_);
  if (status != Status::SUCCESS) return status;

  // step2: execute
  output_tensor_map_.clear();
  snpe_.get()->execute(input_tensor_map_, output_tensor_map_);

  // step3: process output
  status = ProcessOutput(output_tensor_map_, outputs);
  return status;
}

Status SnpeGPUExecutor::Prepare(const char *model_name) {
  static zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::Runtime_t::GPU;
  snpe_ = BuildSnpeRuntime(model_name, runtime);
  if (snpe_ == nullptr) return Status::RUNTIME_ERROR;
  return Status::SUCCESS;
}

Status SnpeGPUExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  Status status;
  // step1: prepare inputs
  input_tensor_map_.clear();
  status = ProcessInput(snpe_.get(), inputs, &input_tensor_map_);
  if (status != Status::SUCCESS) return status;

  // step2: execute
  output_tensor_map_.clear();
  snpe_.get()->execute(input_tensor_map_, output_tensor_map_);

  // step3: process output
  status = ProcessOutput(output_tensor_map_, outputs);
  return status;
}

Status SnpeDSPExecutor::Prepare(const char *model_name) {
  static zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::Runtime_t::DSP;
  snpe_ = BuildSnpeRuntime(model_name, runtime);
  if (snpe_ == nullptr) return Status::RUNTIME_ERROR;
  return Status::SUCCESS;
}

Status SnpeDSPExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  Status status;
  // step1: prepare inputs
  input_tensor_map_.clear();
  status = ProcessInput(snpe_.get(), inputs, &input_tensor_map_);
  if (status != Status::SUCCESS) return status;

  // step2: execute
  output_tensor_map_.clear();
  snpe_.get()->execute(input_tensor_map_, output_tensor_map_);

  // step3: process output
  status = ProcessOutput(output_tensor_map_, outputs);
  return status;
}


}  // namespace nnbench
