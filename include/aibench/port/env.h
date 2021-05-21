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

#ifndef AIBENCH_PORT_ENV_H_
#define AIBENCH_PORT_ENV_H_

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <malloc.h>
#endif

#include <sys/stat.h>
#include "aibench/public/aibench.h"

namespace aibench {
namespace port {

class MallocLogger {
 public:
  MallocLogger() = default;
  virtual ~MallocLogger() = default;
};

class FileSystem;
class LogWriter;

class Env {
 public:
  virtual int64_t NowMicros() = 0;
  virtual Status AdviseFree(void *addr, size_t length);
  virtual Status GetCPUMaxFreq(std::vector<float> *max_freqs);
  virtual Status SchedSetAffinity(const std::vector<size_t> &cpu_ids);
  virtual FileSystem *GetFileSystem() = 0;
  virtual LogWriter *GetLogWriter() = 0;
  // Return the current backtrace, will allocate memory inside the call
  // which may fail
  virtual std::vector<std::string> GetBackTraceUnsafe(int max_steps) = 0;
  virtual std::unique_ptr<MallocLogger> NewMallocLogger(
      std::ostringstream *oss,
      const std::string &name);

  static Env *Default();
};

}  // namespace port

inline int64_t NowMicros() {
  return port::Env::Default()->NowMicros();
}

inline Status AdviseFree(void *addr, size_t length) {
  return port::Env::Default()->AdviseFree(addr, length);
}

inline Status GetCPUMaxFreq(std::vector<float> *max_freqs) {
  return port::Env::Default()->GetCPUMaxFreq(max_freqs);
}

inline Status SchedSetAffinity(const std::vector<size_t> &cpu_ids) {
  return port::Env::Default()->SchedSetAffinity(cpu_ids);
}

inline port::FileSystem *GetFileSystem() {
  return port::Env::Default()->GetFileSystem();
}

inline Status Memalign(void **memptr, size_t alignment, size_t size) {
#ifdef _WIN32
  *memptr = _aligned_malloc(size, alignment);
  if (*memptr == nullptr) {
    return Status::OUT_OF_RESOURCES;
  } else {
    return Status::SUCCESS;
  }
#else
#if defined(__ANDROID__) || defined(__hexagon__)
  *memptr = memalign(alignment, size);
  if (*memptr == nullptr) {
    return Status::OUT_OF_RESOURCES;
  } else {
    return Status::SUCCESS;
  }
#else
  int error = posix_memalign(memptr, alignment, size);
  if (error != 0) {
    if (*memptr != nullptr) {
      free(*memptr);
      *memptr = nullptr;
    }
    return Status::OUT_OF_RESOURCES;
  } else {
    return Status::SUCCESS;
  }
#endif
#endif
}

inline Status GetEnv(const char *name, std::string *value) {
#ifdef _WIN32
  char *val;
  size_t len;
  errno_t error = _dupenv_s(&val, &len, name);
  if (error != 0) {
    return Status::RUNTIME_ERROR;
  } else {
    if (val != nullptr) {
      *value = std::string(val);
      free(val);
    }
    return Status::SUCCESS;
  }
#else
  char *val = getenv(name);
  if (val != nullptr) {
    *value = std::string(val);
  }
  return Status::SUCCESS;
#endif
}

#if defined(_WIN32) && !defined(S_ISREG)
#define S_ISREG(m) (((m) & 0170000) == (0100000))
#endif
}  // namespace aibench

#endif  // AIBENCH_PORT_ENV_H_
