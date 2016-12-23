/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef RADIANCE__H
#define RADIANCE__H

#include "component.h"
#include "memory.h"
#include "pipeline.h"
#include "schema.h"
#include "system.h"
#include "table.h"

namespace radiance {

struct Universe {
  DataManager data_manager;
  ComponentManager component_manager;
  SystemManager system_manager;
};

extern Universe* universe;

Status start(Universe* u);

}

#endif
