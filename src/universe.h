#ifndef UNIVERSE__H
#define UNIVERSE__H

#include "component.h"
#include "program.h"

namespace radiance {

// Holds the game engine state.
struct Universe {
  DataManager data_manager;
  ComponentManager component_manager;
  SystemManager system_manager;

  Program program;
};

}  // namespace radiance

#endif  // UNIVERSE__H
