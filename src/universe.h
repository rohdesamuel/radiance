#ifndef UNIVERSE__H
#define UNIVERSE__H

#include "program.h"
#include "source_manager.h"

namespace radiance {

// Holds the game engine state.
struct Universe {
  SourceManager* source_manager;
};

}  // namespace radiance

#endif  // UNIVERSE__H
