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

typedef void (*Insert)(struct Collection*, void* element);
typedef void (*Update)(struct Collection*, void* element);
typedef void (*Remove)(struct Collection*, void* key);
typedef bool (*Iterate)(struct Collection*, struct Stack*, void* state);

struct Collection {
  void* data;
  uint64_t count;
  size_t key_size;
  size_t value_size;

  Insert insert;
  Update update;
  Remove remove;
  Iterate iterate;
};

typedef bool (*Select)(uint8_t, ...);
typedef void (*Read)(struct Collection*, ...);
typedef void (*Write)(struct Collection*, ...);

struct Pipeline {
  const Id id;
  uint8_t priority;

  Select select;
  Read read;
  Write write;
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
