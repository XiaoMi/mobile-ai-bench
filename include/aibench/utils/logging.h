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

#ifndef AIBENCH_UTILS_LOGGING_H_
#define AIBENCH_UTILS_LOGGING_H_

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "aibench/public/aibench.h"
#include "aibench/port/env.h"
#include "aibench/port/logger.h"
#include "aibench/utils/macros.h"
#include "aibench/utils/memory.h"
#include "aibench/utils/string_util.h"


namespace aibench {

namespace logging_internal {

#define LOG(severity) \
  ::aibench::port::Logger(__FILE__, __LINE__, aibench::severity)

#define LOG_PTR(severity) \
  make_unique<aibench::port::Logger>(__FILE__, __LINE__, aibench::severity)

#define VLOG_IS_ON(vll) (aibench::ShouldGenerateVLogMessage(vll))
#define VLOG(vll) if (VLOG_IS_ON(vll)) LOG(INFO)

// AIBENCH_CHECK/AIBENCH_ASSERT dies with a fatal error if condition is not true.
// AIBENCH_ASSERT is controlled by NDEBUG ('-c opt' for bazel) while AIBENCH_CHECK
// will be executed regardless of compilation mode.
// Therefore, it is safe to do things like:
//    AIBENCH_CHECK(fp->Write(x) == 4)
//    AIBENCH_CHECK(fp->Write(x) == 4, "Write failed")
// which are not safe for AIBENCH_ASSERT.
#define AIBENCH_CHECK(condition, ...) \
  if (!(condition)) \
  LOG(FATAL) << "Check failed: " #condition " " << aibench::MakeString(__VA_ARGS__)

#ifndef NDEBUG
#define AIBENCH_ASSERT(condition, ...) \
  if (!(condition)) \
  LOG(FATAL) << "Assert failed: " #condition " " \
             << aibench::MakeString(__VA_ARGS__)
#else
#define AIBENCH_ASSERT(condition, ...) ((void)0)
#endif

template <typename T>
T &&CheckNotNull(const char *file, int line, const char *exprtext, T &&t) {
  if (t == nullptr) {
    ::aibench::port::Logger(file, line, FATAL) << std::string(exprtext);
  }
  return std::forward<T>(t);
}

#define AIBENCH_CHECK_NOTNULL(val) \
  ::aibench::logging_internal::CheckNotNull(__FILE__, __LINE__, \
                                         "'" #val "' Must not be NULL", (val))

#define AIBENCH_NOT_IMPLEMENTED AIBENCH_CHECK(false, "not implemented")

#define AIBENCH_CHECK_SUCCESS(stmt)                             \
  {                                                          \
    Status status = (stmt);                              \
    if (status != Status::SUCCESS) {                \
      LOG(FATAL) << #stmt << " failed with error: "          \
              << status.information();                       \
    }                                                        \
  }

#define AIBENCH_RETURN_IF_ERROR(stmt)                           \
  {                                                          \
    Status status = (stmt);                              \
    if (status != Status::SUCCESS) {                \
      VLOG(0) << #stmt << " failed with error: "             \
              << status.information();                       \
      return status;                                         \
    }                                                        \
  }

class LatencyLogger {
 public:
  LatencyLogger(int vlog_level, const std::string &message)
      : vlog_level_(vlog_level), message_(message) {
    if (VLOG_IS_ON(vlog_level_)) {
      start_micros_ = NowMicros();
      VLOG(vlog_level_) << message_ << " started";
    }
  }
  ~LatencyLogger() {
    if (VLOG_IS_ON(vlog_level_)) {
      int64_t stop_micros = NowMicros();
      VLOG(vlog_level_) << message_
                        << " latency: " << stop_micros - start_micros_ << " us";
    }
  }

 private:
  const int vlog_level_;
  const std::string message_;
  int64_t start_micros_;

  AIBENCH_DISABLE_COPY_AND_ASSIGN(LatencyLogger);
};

#define AIBENCH_LATENCY_LOGGER(vlog_level, ...)                                  \
  aibench::logging_internal::LatencyLogger latency_logger_##__line__(            \
      vlog_level, VLOG_IS_ON(vlog_level) ? aibench::MakeString(__VA_ARGS__) : "")


#ifdef AIBENCH_ENABLE_MALLOC_LOGGING
#define AIBENCH_MEMORY_LOGGING_GUARD()                                      \
  auto malloc_logger_##__line__ = port::Env::Default()->NewMallocLogger( \
      ::aibench::port::Logger(__FILE__, __LINE__, aibench::INFO), \
      std::string(__FILE__) + ":" + std::string(__func__));
#else
#define AIBENCH_MEMORY_LOGGING_GUARD()
#endif

}  // namespace logging_internal
}  // namespace aibench

#endif  // AIBENCH_UTILS_LOGGING_H_
