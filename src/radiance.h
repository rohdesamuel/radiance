/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef RADIANCE__H
#define RADIANCE__H

#include "universe.h"

namespace radiance {

Status start(Universe* u);
Status stop(Universe* u);

namespace program {

struct Options {
  uint8_t cores = 1;
};

Status create_program(Options opts, Program** program);
Status attach_process(Program* program, Process* process);

}  // namespace program

}  // namespace radiance

#endif  // #ifndef RADIANCE__H
