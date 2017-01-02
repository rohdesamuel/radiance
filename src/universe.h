#ifndef UNIVERSE__H
#define UNIVERSE__H

#include "component.h"

namespace radiance {

struct Universe {
  DataManager data_manager;
  ComponentManager component_manager;
  SystemManager system_manager;
};

}  // namespace radiance

#endif  // UNIVERSE__H
