// Copyright 2021 The AIBENCH Authors. All Rights Reserved.
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

#ifndef AIBENCH_PORT_LINUX_ENV_H_
#define AIBENCH_PORT_LINUX_ENV_H_

#include <string>
#include <vector>

#include "aibench/port/linux_base/env.h"
#include "aibench/port/logger.h"

namespace aibench {
namespace port {

class LinuxEnv : public LinuxBaseEnv {
 public:
  Status SchedSetAffinity(const std::vector<size_t> &cpu_ids) override;
  LogWriter *GetLogWriter() override;
  std::vector<std::string> GetBackTraceUnsafe(int max_steps) override;

 private:
  LogWriter log_writer_;
};

}  // namespace port
}  // namespace aibench

#endif  // AIBENCH_PORT_LINUX_ENV_H_
