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

#ifndef AIBENCH_PORT_POSIX_FILE_SYSTEM_H_
#define AIBENCH_PORT_POSIX_FILE_SYSTEM_H_

#include <string>
#include <memory>

#include "aibench/port/file_system.h"

namespace aibench {
namespace port {

class PosixFileSystem : public FileSystem {
 public:
  PosixFileSystem() = default;
  ~PosixFileSystem() override = default;
  Status NewReadOnlyMemoryRegionFromFile(const char *fname,
      std::unique_ptr<ReadOnlyMemoryRegion>* result) override;
};

}  // namespace port
}  // namespace aibench

#endif  // AIBENCH_PORT_POSIX_FILE_SYSTEM_H_
