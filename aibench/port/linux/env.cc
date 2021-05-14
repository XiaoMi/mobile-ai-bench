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

#include "aibench/port/linux/env.h"

#include <execinfo.h>
#include <sys/time.h>

#include <cstddef>
#include <string>
#include <vector>

#include "aibench/port/env.h"
#include "aibench/port/posix/backtrace.h"
#include "aibench/port/posix/file_system.h"
#include "aibench/port/posix/time.h"
#include "aibench/utils/macros.h"

namespace aibench {
namespace port {

// In our embedded linux device, SchedSetAffinity has side effects
// on performance, so we override this method to do nothing. You
// can try to comment this function, perhaps you could get a better
// performance as we do in Android devices.
Status LinuxEnv::SchedSetAffinity(const std::vector<size_t> &cpu_ids) {
  AIBENCH_UNUSED(cpu_ids);

  return Status::SUCCESS;
}

LogWriter *LinuxEnv::GetLogWriter() {
  return &log_writer_;
}

std::vector<std::string> LinuxEnv::GetBackTraceUnsafe(int max_steps) {
  return aibench::port::posix::GetBackTraceUnsafe(max_steps);
}

Env *Env::Default() {
  static LinuxEnv linux_env;
  return &linux_env;
}

}  // namespace port
}  // namespace aibench
