/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef RADIANCE__H
#define RADIANCE__H

#include "common.h"
#include "universe.h"

BEGIN_EXTERN_C

namespace radiance {

struct Element {
  uint8_t* data;
  size_t size;
};

enum class MutateBy {
  UNKNOWN = 0,
  INSERT,
  UPDATE,
  REMOVE
};

struct Mutation {
  Element* element;
  MutateBy mutate_by;
};

typedef bool (*Select)(uint8_t, ...);
typedef Mutation (*Transform)(struct Stack*);

typedef void (*Mutate)(struct Collection*, const Mutation*);

// Put the next element onto the stack. Gives nullptr as state to begin. Return
// nullptr if at end.
typedef uint8_t* (*Iterate)(struct Collection*, struct Stack*, uint8_t* state);

struct Collection {
  const Id id;
  uint8_t* self;

  uint64_t count;
  size_t key_size;
  size_t value_size;
  size_t state_size;

  Mutate mutate;
  Iterate iterate;
};

struct Pipeline {
  const Id id;
  uint8_t priority;

  Select select;
  Transform transform;
};

struct Program {
  void* self;
};

Universe* universe();

Status::Code init(Universe* universe);
Status::Code start();
Status::Code stop();
Status::Code loop();

Id create_program(const char* name);
Pipeline* add_pipeline(const char* program, const char* source, const char* sink);
Collection* add_collection(const char* program, const char* name);

Status::Code add_source(Pipeline*, const char* collection);
Status::Code add_sink(Pipeline*, const char* collection);

Status::Code share_collection(const char* source, const char* dest);
Status::Code copy_collection(const char* source, const char* dest);

}  // namespace radiance

END_EXTERN_C

#endif  // #ifndef RADIANCE__H
