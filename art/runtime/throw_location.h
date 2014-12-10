/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ART_RUNTIME_THROW_LOCATION_H_
#define ART_RUNTIME_THROW_LOCATION_H_

#include "base/macros.h"
#include "root_visitor.h"

#include <stdint.h>
#include <string>

namespace art {

namespace mirror {
class ArtMethod;
class Object;
}  // mirror

class PACKED(4) ThrowLocation {
 public:
  ThrowLocation() {
    Clear();
  }

  ThrowLocation(mirror::Object* throw_this_object, mirror::ArtMethod* throw_method,
                uint32_t throw_dex_pc) :
      this_object_(throw_this_object),
      method_(throw_method),
      dex_pc_(throw_dex_pc) {}

  mirror::Object* GetThis() const {
    return this_object_;
  }

  mirror::ArtMethod* GetMethod() const {
    return method_;
  }

  uint32_t GetDexPc() const {
    return dex_pc_;
  }

  void Clear() {
    this_object_ = NULL;
    method_ = NULL;
    dex_pc_ = -1;
  }

  std::string Dump() const SHARED_LOCKS_REQUIRED(Locks::mutator_lock_);

  void VisitRoots(RootVisitor* visitor, void* arg);

 private:
  // The 'this' reference of the throwing method.
  mirror::Object* this_object_;
  // The throwing method.
  mirror::ArtMethod* method_;
  // The instruction within the throwing method.
  uint32_t dex_pc_;
};

}  // namespace art

#endif  // ART_RUNTIME_THROW_LOCATION_H_
