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


#ifndef NNBENCH_EXECUTORS_SNPE_SNPE_UTILS_H_
#define NNBENCH_EXECUTORS_SNPE_SNPE_UTILS_H_

#include <memory>
#include <vector>

#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/IUDL.hpp"
#include "DlSystem/UDLContext.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/UDLFunc.hpp"
#include "SNPE/SNPE.hpp"

namespace nnbench {
namespace snpe_utils {

std::unique_ptr<zdl::DlContainer::IDlContainer> LoadContainerFromFile(
    const char *containerPath);

std::unique_ptr<zdl::SNPE::SNPE> SetBuilderOptions(
    const std::unique_ptr<zdl::DlContainer::IDlContainer> &container,
    zdl::DlSystem::Runtime_t runtime,
    zdl::DlSystem::UDLBundle udlBundle,
    bool useUserSuppliedBuffers);

}  // namespace snpe_utils
}  // namespace nnbench

#endif  // NNBENCH_EXECUTORS_SNPE_SNPE_UTILS_H_
