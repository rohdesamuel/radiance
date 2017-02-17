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
  REMOVE,
  INSERT_OR_UPDATE,
};

struct Mutation {
  MutateBy mutate_by;
  uint8_t* element;
};

typedef bool (*Select)(uint8_t, ...);
typedef void (*Transform)(struct Stack*);

typedef void (*Mutate)(struct Collection*, const Mutation*);
typedef void (*Copy)(const uint8_t* value, const uint8_t* key, struct Stack*);

// Put the next element onto the stack. Gives nullptr as state to begin. Return
// nullptr if at end.
typedef uint8_t* (*Iterate)(struct Collection*, struct Stack*, uint8_t* state);

struct Iterator {
  uint8_t* data;
  uint32_t offset;
  size_t size;
};

struct Collection {
  const Id id;
  uint8_t* self;

  Iterator keys;
  Iterator values;
  
  uint64_t (*count)(Collection*);

  Copy copy;
  Mutate mutate;
};

struct Pipeline {
  const Id id;
  const Id program;

  uint8_t priority;

  Select select;
  Transform transform;
};

struct Program {
  const Id id;
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
