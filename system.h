/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef SYSTEM__H
#define SYSTEM__H

#include <functional>
#include <boost/container/list.hpp>

#include "common.h"

namespace radiance
{

class Frame {
private:
	StackMemory<512> stack;
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
		DEBUG_ASSERT(ret_, error::Codes::NULL_POINTER);
		return (T*)ret_;
	}

	template<class T>
	void clear_result_as() {
		DEBUG_ASSERT(ret_, error::Codes::NULL_POINTER);
		((T*)ret_)->~T();
		stack.free<T>((T*)ret_);
		ret_ = nullptr;
	}

	void clear() {
		stack.clear();
	}
};

class System {
private:
	std::function<void(Frame*)> f_;

public:
	System() {}

	template<class F>
	System(F f) : f_(f) {}

	inline void operator()(Frame* f) const {
		f_(f);
	}

	System operator*(System system) {
		return [=](Frame* frame) {
			system(frame);
			f_(frame);
		};
	}
};

typedef boost::container::list<std::function<void(void)>> SystemQueue;

}

#endif
