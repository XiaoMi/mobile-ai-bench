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

#include "nnbench/executors/snpe/snpe_utils.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <iterator>
#include <vector>
#include <string>
#include <numeric>
#include <functional>

#include "DlContainer/IDlContainer.hpp"
#include "SNPE/SNPEBuilder.hpp"

namespace nnbench {
namespace snpe_utils {

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

}  // namespace snpe_utils
}  // namespace nnbench
