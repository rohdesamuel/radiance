#include "radiance.h"

namespace radiance {

static Universe* universe = nullptr;

int32_t start(Universe* u, Program* p) {
  universe = u;
  return Status::OK;
}

int32_t stop(Universe* u) {
  universe = nullptr;
  return Status::OK;
}

}  // namespace radiance
