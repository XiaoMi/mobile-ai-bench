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

#ifndef AIBENCH_PORT_ANDROID_ENV_H_
#define AIBENCH_PORT_ANDROID_ENV_H_

#include <memory>
#include <string>
#include <vector>

#include "aibench/port/android/logger.h"
#include "aibench/port/env.h"
#include "aibench/port/linux_base/env.h"
#include "aibench/port/posix/file_system.h"

namespace aibench {
namespace port {

class AndroidEnv : public LinuxBaseEnv {
 public:
  LogWriter *GetLogWriter() override;
  Status GetCPUMaxFreq(std::vector<float> *max_freqs) override;
  std::vector<std::string> GetBackTraceUnsafe(int max_steps) override;
  std::unique_ptr<MallocLogger> NewMallocLogger(
      std::ostringstream *oss,
      const std::string &name) override;

 private:
  AndroidLogWriter log_writer_;
};

}  // namespace port
}  // namespace aibench

#endif  // AIBENCH_PORT_ANDROID_ENV_H_
