#ifndef UNIVERSE__H
#define UNIVERSE__H

#include "program.h"

namespace radiance {

// Holds the game engine state.
struct Universe {
  Program* program;
};

}  // namespace radiance

#endif  // UNIVERSE__H
