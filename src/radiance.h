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
#include "universe.h"

namespace radiance {

extern Universe* universe;

Status start(Universe* u);
Status stop(Universe* u);

}

#endif
