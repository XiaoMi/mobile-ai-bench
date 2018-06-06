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

#include "nnbench/executors/snpe/snpe_utils.h"
#include "DiagLog/IDiagLog.hpp"
#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/ITensorFactory.hpp"
#include "DlSystem/UDLFunc.hpp"
#include "DlSystem/IUserBuffer.hpp"
#include "SNPE/SNPE.hpp"
#include "SNPE/SNPEFactory.hpp"

namespace nnbench {

Status SnpeCPUExecutor::Prepare(const char *model_name) {
  zdl::DlSystem::UDLBundle udlBundle;
  // 0xdeadbeaf to test cookie
  udlBundle.cookie = reinterpret_cast<void *>(0xdeadbeaf);

  static zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::Runtime_t::CPU;
  if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime)) {
    std::cerr << "SNPE Runtime" << static_cast<int>(runtime)
              << " not available! " << std::endl;
    return Status::RUNTIME_ERROR;
  }

  std::unique_ptr<zdl::DlContainer::IDlContainer>
      container = nnbench::snpe_utils::LoadContainerFromFile(model_name);
  snpe_ = nnbench::snpe_utils::SetBuilderOptions(
      container, runtime, udlBundle, false);

  return Status::SUCCESS;
}

Status SnpeCPUExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  // step1: prepare inputs
  const auto &input_tensor_names_ref = snpe_.get()->getInputTensorNames();
  const auto &input_tensor_names = *input_tensor_names_ref;
  if (inputs.size() != input_tensor_names.size()) {
    std::cerr << "inputs size not matched" << std::endl;
    return Status::RUNTIME_ERROR;
  }
  std::unique_ptr<zdl::DlSystem::ITensor> input_tensor;
  for (size_t i = 0; i < input_tensor_names.size(); i++) {
    std::string input_name(input_tensor_names.at(i));
    const auto &input_shape_opt =
        snpe_.get()->getInputDimensions(input_tensor_names.at(i));
    const auto &input_shape = *input_shape_opt;
    input_tensor =
        zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(input_shape);
    size_t input_size = 1;
    for (size_t j = 0; j < input_shape.rank(); ++j)
      input_size *= input_shape[j];
    std::vector<float> input_vec(inputs.at(input_name.c_str()).data().get(),
                                 inputs.at(input_name).data().get()
                                     + inputs.at(input_name).size());

    std::copy(input_vec.begin(), input_vec.end(), input_tensor.get()->begin());
    input_tensor_map_.add(input_name.c_str(), input_tensor.release());
  }

  // step2: execute
  snpe_.get()->execute(input_tensor_map_, output_tensor_map_);

  // step3: process output TODO(wuchenghui)
  (void) (outputs);

  return Status::SUCCESS;
}

Status SnpeGPUExecutor::Prepare(const char *model_name) {
  (void) (model_name);
  float sum = 0;
  for (int i = 0; i < 1000000; ++i) {
    sum += i;
  }
  return Status::SUCCESS;
}

Status SnpeGPUExecutor::Run(const std::map<std::string, BaseTensor> &inputs,
                            std::map<std::string, BaseTensor> *outputs) {
  (void) (inputs);
  (void) (outputs);
  float sum = 0;
  for (int i = 0; i < 10000000; ++i) {
    sum += i;
  }
  return Status::SUCCESS;
}

}  // namespace nnbench
