/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef SYSTEM__H
#define SYSTEM__H

#include <functional>
#include <vector>
#include <iostream>

#include "common.h"
#include "stack_memory.h"

namespace radiance
{

class Frame {
private:
  StackMemory<128> stack;
  void* ret_ = nullptr;

  template<class T>
  T* push(const T& t) {
    T* new_t = (T*)stack.alloc(sizeof(T));
    new (new_t) T(t);
    return new_t;
  }

  template<class T>
  T* push(T&& t) {
    T* new_t = (T*)stack.alloc(sizeof(T));
    new (new_t) T(std::move(t));
    return new_t;
  }

public:
  template<class T>
  T* alloc() {
    return (T*)stack.alloc(sizeof(T));
  }

  template<class T>
  T* result(const T& t) {
    return (T*)(ret_ = (T*)push(t));
  }

  template<class T>
  T* result(T&& t) {
    return (T*)(ret_ = (T*)push(std::move(t)));
  }

  template<class T>
  inline T* result() const {
    DEBUG_ASSERT(ret_, Status::Code::NULL_POINTER);
    return (T*)ret_;
  }

  template<class T>
  void clear_result_as() {
    DEBUG_ASSERT(ret_, Status::Code::NULL_POINTER);
    ((T*)ret_)->~T();
    stack.free<T>((T*)ret_);
    ret_ = nullptr;
  }

  void clear() {
    stack.clear();
  }
};
#if 0
class System {
private:
  std::function<void(Frame*)> f_;

public:
  System() {}

  template<class F>
  System(F f) : f_(f) {}

  template<class F, class... State>
  System(F f, State... state) {
    f_ = [=](Frame* frame) {
      return f(frame, state...);
    };
  }

  inline void operator()(Frame* f) const {
    f_(f);
  }

  System operator*(System system) {
    std::function<void(Frame*)>& my_system = f_;
    return System([my_system, system](Frame* frame) {
      system(frame);
      my_system(frame);
    });
  }
};
#endif

class System {
private:
  typedef std::function<void(Frame*)> Function;
  Function f_;

public:
  System() {}

  template<class F>
  System(F f) : f_(f) {}

  template<class F, class... State>
  System(F f, State... state) {
    f_ = [=](Frame* frame) {
      return f(frame, state...);
    };
  }

  inline void operator()(Frame* frame) const {
    f_(frame);
  }
};

typedef std::vector<System> SystemList;

class SystemExecutor {
 public:
  struct Element {
    Id id;
    System system;
  };

  typedef std::vector<Element> Systems;
  typedef typename Systems::iterator iterator;
  typedef typename Systems::const_iterator const_iterator;

  SystemExecutor() {}
  SystemExecutor(System back): back_(back) {}

  void operator()(Frame* frame) const {
    for(auto& el : systems_) {
      el.system(frame);
    }
    back_(frame);
  }

  std::vector<Id> push(std::vector<System> systems) {
    std::vector<Id> ret;
    ret.reserve(systems.size());
    for(System& system : systems) {
      ret.push_back(push(system));
    }
    return ret;
  }

  Id push(System system) {
    Id new_id = id_++;  
    systems_.push_back({new_id, system});
    return new_id;
  }

  System& back() {
    return back_;
  }

  void erase(iterator it) {
    systems_.erase(it);
  }

  void erase(Id id) {
    iterator it = begin();
    while(it != end()) {
      if (it->id == id) {
        break;
      }
    }
  }

  iterator begin() {
    return systems_.begin();
  }

  iterator end() {
    return systems_.end();
  }

  const_iterator begin() const {
    return systems_.begin();
  }

  const_iterator end() const {
    return systems_.end();
  }

 private:
  Systems systems_;
  System back_ = [](Frame*){};
  Id id_;
};

}

#endif
