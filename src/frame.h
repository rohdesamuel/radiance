#ifndef FRAME__H
#define FRAME__H

#include <functional>

#include "common.h"
#include "stack_memory.h"

namespace radiance
{

class Frame {
private:
  StackMemory<128> stack_;
  void* ret_ = nullptr;

public:
  template<typename Type_>
  void pop() {
    std::cout << "<pop> " << ret_ << std::endl;
    ret_ = stack_.pop(sizeof(Type_));
    std::cout << "</pop> " << ret_ << std::endl;
  }

  template<typename Type_>
  Type_* push(const Type_& t) {
    Type_* new_t = (Type_*)stack_.alloc(sizeof(Type_));
    new (new_t) Type_(t);
    return new_t;
  }

  template<typename Type_>
  Type_* push(Type_&& t) {
    Type_* new_t = (Type_*)stack_.alloc(sizeof(Type_));
    new (new_t) Type_(std::move(t));
    return new_t;
  }

  template<typename Type_>
  Type_* alloc() {
    return (Type_*)stack_.alloc(sizeof(Type_));
  }

  template<typename Type_>
  Type_* result(const Type_& t) {
    return (Type_*)(ret_ = (Type_*)push(t));
  }

  template<typename Type_>
  Type_* result(Type_&& t) {
    return (Type_*)(ret_ = (Type_*)push(std::move(t)));
  }

  template<typename Type_>
  inline Type_* result() const {
    DEBUG_ASSERT(ret_, Status::Code::NULL_POINTER);
    return (Type_*)ret_;
  }

  void clear() {
    stack_.clear();
  }
};

}  // namespace radiance

#endif  // FRAME__H
