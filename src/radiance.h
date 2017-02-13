/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef RADIANCE__H
#define RADIANCE__H

#include "universe.h"

BEGIN_EXTERN_C

namespace radiance {
struct Args {
  size_t arg_size;
  uint8_t* args;
};

typedef bool (*Select)(uint8_t, ...);
typedef void (*Read)(struct Collection*, ...);
typedef void (*Write)(struct Collection*, ...);

struct _Pipeline {
  struct Collection* source = nullptr;
  struct Collection* sink = nullptr;

  Stack stack;
  Select select;
  Read read;
  Write write;
};

int32_t start(Universe* u);
int32_t stop(Universe* u);
Universe* universe();

namespace program {

struct Options {
  uint8_t cores = 1;
};

Status::Code create_program(Options opts, Program** program);
Status::Code attach_process(Program* program, Process* process);

}  // namespace program

}  // namespace radiance

END_EXTERN_C

#endif  // #ifndef RADIANCE__H
