#include "radiance.h"

namespace radiance {

Status start(Universe* u) {
  universe = u;
  return Status::OK;
}

Status stop(Universe*) {
  universe = nullptr;
  return Status::OK;
}

}
