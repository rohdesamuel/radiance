#include "radiance.h"

namespace radiance {

static Universe* universe_ = nullptr;

int32_t start(Universe* u) {
  universe_ = u;

  u->source_manager = new SourceManager();

  return Status::OK;
}

int32_t stop() {

  delete universe_->source_manager;

  universe_ = nullptr;
  return Status::OK;
}

Universe* universe() {
  return universe_;
}

}  // namespace radiance
