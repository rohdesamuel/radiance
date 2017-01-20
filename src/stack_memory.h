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

template<size_t max_size>
class StackMemory {
private:
  struct StackFrame {
    size_t size;
  };

  uint8_t stack_[max_size];
  // Always points to the next empty piece of memory.
  uint8_t* top_;
  uint8_t* ceiling_;

public:
  StackMemory() : top_(stack_), ceiling_(stack_ + max_size) {}
  ~StackMemory() {
    clear();
  }

  void* alloc(size_t type_size) {
    void* ret = nullptr;

    StackFrame frame{type_size};
    uint8_t* new_top = top_ + type_size + sizeof(StackFrame);
    if (new_top <= ceiling_) {
      *(StackFrame*)(new_top - sizeof(StackFrame)) = frame;
      ret = top_;
      top_ = new_top;
    }

    DEBUG_ASSERT(ret, Status::Code::MEMORY_OUT_OF_BOUNDS);
    return ret;
  }

  // Return pointer to value that is on the top of the stack.
  void* top() const {
    void* ret = nullptr;

    StackFrame* frame = (StackFrame*)(top_ - sizeof(StackFrame));
    if ((uint8_t*)(frame) >= stack_) {
      ret = top_ - sizeof(StackFrame) - frame->size;
    }

    return ret;
  }

  void free() {
    StackFrame* frame = (StackFrame*)(top_ - sizeof(StackFrame));
    if ((uint8_t*)(frame) >= stack_) {
      top_ = top_ - sizeof(StackFrame) - frame->size;
    }
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
