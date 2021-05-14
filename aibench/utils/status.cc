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

#include <sstream>

#include "aibench/utils/memory.h"
#include "aibench/public/aibench.h"

namespace aibench {

class Status::Impl {
 public:
  explicit Impl(const Code code): code_(code), information_("") {}
  Impl(const Code code, const std::string &informaton)
      : code_(code), information_(informaton) {}
  ~Impl() = default;

  void SetCode(const Code code) { code_ = code; }
  Code code() const { return code_; }
  void SetInformation(const std::string &info) { information_ = info; }
  std::string information() const {
    if (information_.empty()) {
      return CodeToString();
    } else {
      return CodeToString() + ": " + information_;
    }
  }

 private:
  std::string CodeToString() const {
    switch (code_) {
      case Status::SUCCESS:
        return "Success";
      case Status::INVALID_ARGS:
        return "Invalid Arguments";
      case Status::OUT_OF_RESOURCES:
        return "Out of resources";
      case UNSUPPORTED:
        return "Unsupported";
      case RUNTIME_ERROR:
        return "Runtime error";
      default:
        std::ostringstream os;
        os << code_;
        return os.str();
    }
  }

 private:
  Status::Code code_;
  std::string information_;
};

Status::Status()
    : impl_(new Status::Impl(Status::SUCCESS)) {}
Status::Status(const Code code) : impl_(new Status::Impl(code)) {}
Status::Status(const Code code, const std::string &information)
    : impl_(new Status::Impl(code, information)) {}
Status::Status(const Status &other)
    : impl_(new Status::Impl(other.code(), other.information())) {}
Status::Status(Status &&other)
    : impl_(new Status::Impl(other.code(), other.information())) {}
Status::~Status() = default;

Status& Status::operator=(const Status &other) {
  impl_->SetCode(other.code());
  impl_->SetInformation(other.information());
  return *this;
}
Status& Status::operator=(const Status &&other) {
  impl_->SetCode(other.code());
  impl_->SetInformation(other.information());
  return *this;
}

Status::Code Status::code() const {
  return impl_->code();
}

std::string Status::information() const {
  return impl_->information();
}

bool Status::operator==(const Status &other) const {
  return other.code() == impl_->code();
}

bool Status::operator!=(const Status &other) const {
  return other.code() != impl_->code();
}

}  // namespace aibench
