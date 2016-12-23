#include "radiance.h"

namespace radiance {

Universe* universe = nullptr; 

Status start(Universe* u) {
  universe = u;
  return Status::OK;
}

}
