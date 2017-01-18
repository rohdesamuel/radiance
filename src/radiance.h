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

struct Source {
  void (*reader)(Source*, ...);
  size_t arg_size;
  uint8_t* args;
};

struct Sink {
  void (*writer)(Sink*, ...);
  size_t arg_size;
  uint8_t* args;
};

int32_t reader(Source* source, Args* args);

struct Pipe {
  Source* source = nullptr;
  Sink* sink = nullptr;
  void run() {
    if (source && sink) {

    }
  }
};


int32_t start(Universe* u, Program* p);
int32_t stop(Universe* u);

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
