/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef STACK_MEMORY__H
#define STACK_MEMORY__H

#include "common.h"
#include <memory.h>

namespace radiance
{

template<size_t size>
class StackMemory {
private:
  uint8_t stack_[size];
  uint8_t* top_;

public:
  StackMemory() : top_(stack_) {}
  ~StackMemory() {
    clear();
  }

  void* alloc(size_t type_size) {
    void* ret = nullptr;

    if (type_size + top_ <= stack_ + size) {
      ret = top_;
      top_ += type_size;
    }

    DEBUG_ASSERT(ret, Status::Code::MEMORY_OUT_OF_BOUNDS);
    return ret;
  }

  template<typename Type_>
  void free(Type_* t) {
    DEBUG_OP(
      memset((void*)t, 0, sizeof(Type_));
    );
  }

  void clear() {
    top_ = stack_;
  }
};

}  // namespace radiance
#endif
