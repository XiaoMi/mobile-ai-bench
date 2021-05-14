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

#ifndef AIBENCH_UTILS_MACROS_H_
#define AIBENCH_UTILS_MACROS_H_

namespace aibench {

// Disable the copy and assignment operator for a class.
#ifndef AIBENCH_DISABLE_COPY_AND_ASSIGN
#define AIBENCH_DISABLE_COPY_AND_ASSIGN(CLASSNAME)     \
  CLASSNAME(const CLASSNAME &) = delete;            \
  CLASSNAME &operator=(const CLASSNAME &) = delete;
#endif

#ifndef AIBENCH_EMPTY_VIRTUAL_DESTRUCTOR
#define AIBENCH_EMPTY_VIRTUAL_DESTRUCTOR(CLASSNAME) \
 public:                                         \
  virtual ~CLASSNAME() {}
#endif

#define AIBENCH_UNUSED(var) (void)(var)

#define AIBENCH_COMPUTE_KERNEL_SOURCE(...) #__VA_ARGS__

// GCC can be told that a certain branch is not likely to be taken (for
// instance, a CHECK failure), and use that information in static analysis.
// Giving it this information can help it optimize for the common case in
// the absence of better information (ie. -fprofile-arcs).
#if defined(COMPILER_GCC3)
#define AIBENCH_PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define AIBENCH_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#else
#define AIBENCH_PREDICT_FALSE(x) (x)
#define AIBENCH_PREDICT_TRUE(x) (x)
#endif

}  // namespace aibench

#endif  // AIBENCH_UTILS_MACROS_H_
