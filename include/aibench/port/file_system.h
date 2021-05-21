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

#ifndef AIBENCH_UTILS_FILE_SYSTEM_H_
#define AIBENCH_UTILS_FILE_SYSTEM_H_

#include <cerrno>
#include <string>
#include <memory>

#include "aibench/public/aibench.h"
#include "aibench/utils/macros.h"

namespace aibench {
namespace port {

class ReadOnlyMemoryRegion {
 public:
  ReadOnlyMemoryRegion() = default;
  virtual ~ReadOnlyMemoryRegion() = default;
  virtual const void *data() const = 0;
  virtual uint64_t length() const = 0;
 private:
  AIBENCH_DISABLE_COPY_AND_ASSIGN(ReadOnlyMemoryRegion);
};

class ReadOnlyBufferMemoryRegion : public ReadOnlyMemoryRegion {
 public:
  ReadOnlyBufferMemoryRegion() : data_(nullptr), length_(0) {}
  ReadOnlyBufferMemoryRegion(const void *data, uint64_t length) :
    data_(data), length_(length) {}
  const void *data() const override { return data_; }
  uint64_t length() const override { return length_; }

 private:
  const void *data_;
  uint64_t length_;
};

class WritableFile {
 public:
  WritableFile() {}
  virtual ~WritableFile();
  virtual Status Append(const char *data, size_t length) = 0;
  virtual Status Close() = 0;
  virtual Status Flush() = 0;
 private:
  AIBENCH_DISABLE_COPY_AND_ASSIGN(WritableFile);
};

class FileSystem {
 public:
  FileSystem() = default;
  virtual ~FileSystem() = default;
  virtual Status NewReadOnlyMemoryRegionFromFile(const char *fname,
      std::unique_ptr<ReadOnlyMemoryRegion>* result) = 0;
};

}  // namespace port
}  // namespace aibench

#endif  // AIBENCH_UTILS_FILE_SYSTEM_H_
