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

// This file defines core AIBENCH APIs.
// There APIs will be stable and backward compatible.

#ifndef AIBENCH_PUBLIC_AIBENCH_H_
#define AIBENCH_PUBLIC_AIBENCH_H_

namespace aibench {

class Status {
 public:
  enum Code {
    SUCCESS = 0,
    RUNTIME_ERROR = 1,
    UNSUPPORTED = 2,
    INVALID_ARGS = 3,
    OUT_OF_RESOURCES = 4,
  };

 public:
  Status();
  Status(const Code code);  // NOLINT(runtime/explicit)
  Status(const Code code, const std::string &information);
  Status(const Status &);
  Status(Status &&);
  Status &operator=(const Status &);
  Status &operator=(const Status &&);
  ~Status();
  Code code() const;
  std::string information() const;

  bool operator==(const Status &other) const;
  bool operator!=(const Status &other) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace aibench

#endif  // AIBENCH_PUBLIC_AIBENCH_H_
