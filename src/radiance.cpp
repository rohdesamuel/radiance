#include "radiance.h"

namespace radiance {

static Universe* universe = nullptr;

Status start(Universe* u) {
  universe = u;
  return Status::OK;
}

Status stop(Universe*) {
  universe = nullptr;
  return Status::OK;
}

}  // namespace radiance
