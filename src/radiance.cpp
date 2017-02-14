#include "radiance.h"
#include "private_universe.h"

#define AS_PRIVATE(expr) ((PrivateUniverse*)(universe_))->expr

namespace radiance {


static Universe* universe_ = nullptr;

Universe* universe() {
  return universe_;
}

Status::Code init(Universe* u) {
  universe_ = u;
  return AS_PRIVATE(init());
}

Status::Code start() {
  return AS_PRIVATE(start());
}

Status::Code stop() {
  universe_ = nullptr;
  return AS_PRIVATE(stop());
}

Status::Code loop() {
  return AS_PRIVATE(loop());
}

Id create_program(const char* name) {
  return AS_PRIVATE(create_program(name));
}

Pipeline* add_pipeline(const char* program, const char* source, const char* sink) {
  return AS_PRIVATE(add_pipeline(program, source, sink));
}

Collection* add_collection(const char* program, const char* name) {
  return AS_PRIVATE(add_collection(program, name));
}

Status::Code add_source(Pipeline* pipeline, const char* source) {
  return AS_PRIVATE(add_source(pipeline, source));
}

Status::Code add_sink(Pipeline* pipeline, const char* sink) {
  return AS_PRIVATE(add_sink(pipeline, sink));
}

Status::Code share_collection(const char* source, const char* dest) {
  return AS_PRIVATE(share_collection(source, dest));
}

Status::Code copy_collection(const char* source, const char* dest) {
  return AS_PRIVATE(copy_collection(source, dest));
}

}  // namespace radiance
