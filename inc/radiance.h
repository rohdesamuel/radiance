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

#ifdef __cplusplus
namespace radiance {
#endif

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
typedef void (*Callback)(struct Pipeline*, ...);

typedef void (*Mutate)(struct Collection*, const struct Mutation*);
typedef void (*Copy)(const uint8_t* key, const uint8_t* value, uint64_t index, struct Stack*);
typedef uint64_t (*Count)(struct Collection*);

struct Iterator {
  uint8_t* data;
  uint32_t offset;
  size_t size;
};

struct Collection {
  const Id id;
  const char* name;

  void* self;

  Iterator keys;
  Iterator values;
  
  Copy copy;
  Mutate mutate;
  Count count;
};

struct Collections {
  uint64_t count;
  struct Collections** collections;
};

struct Pipeline {
  const Id id;
  const Id program;
  const void* self;

  uint8_t priority;

  Select select;
  Transform transform;
};

struct Program {
  const Id id;
  const char* name;
  const void* self;
};

Status::Code init(Universe* universe);
Status::Code start();
Status::Code stop();
Status::Code loop();

Id create_program(const char* name);

struct Pipeline* add_pipeline(const char* program, const char* source, const char* sink);
struct Pipeline* copy_pipeline();
Status::Code remove_pipeline(const char* program, const char* source, const char* sink);

Collection* add_collection(const char* program, const char* name);

Status::Code add_source(struct Pipeline*, const char* collection);
Status::Code add_sink(struct Pipeline*, const char* collection);

Status::Code share_collection(const char* source, const char* dest);
Status::Code copy_collection(const char* source, const char* dest);

#ifdef __cplusplus
}  // namespace radiance
#endif

END_EXTERN_C

#endif  // #ifndef RADIANCE__H
