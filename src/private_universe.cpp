#include "private_universe.h"

#include <algorithm>

namespace {
  std::string form_path(const char* program, const char* resource) {
    return std::string{program} + radiance::NAMESPACE_DELIMETER + std::string{resource};
  }
}  // namespace

namespace radiance {

PrivateUniverse::PrivateUniverse():
    run_state_(RunState::STOPPED) {}

PrivateUniverse::~PrivateUniverse() {}

Status::Code PrivateUniverse::transition(
    RunState allowed, RunState next) {
  return transition(std::vector<RunState>{allowed}, next);
}

Status::Code PrivateUniverse::transition(
    std::vector<RunState>&& allowed, RunState next) {
  if (std::find(allowed.begin(), allowed.end(), run_state_) != allowed.end()) {
    run_state_ = next;
    return Status::OK;
  }
  run_state_ = RunState::ERROR;
  return Status::BAD_RUN_STATE;
}

Status::Code PrivateUniverse::init() {
  return transition(RunState::STOPPED, RunState::INITIALIZED);
}

Status::Code PrivateUniverse::start() {
  return transition(RunState::INITIALIZED, RunState::STARTED);
}

Status::Code PrivateUniverse::loop() {

  return transition({RunState::RUNNING, RunState::STARTED}, RunState::RUNNING);
}

Status::Code PrivateUniverse::stop() {
  return transition({RunState::RUNNING, RunState::UNKNOWN}, RunState::STOPPED);
}

Id PrivateUniverse::create_program(const char* name) {
  return programs_.create_program(name); 
}

Pipeline* PrivateUniverse::add_pipeline(const char* program, const char* source, const char* sink) {
  Program* p = programs_.get_program(program);
  if (!p) {
    return nullptr;
  }

  Collection* src = source ? collections_.get(form_path(program, source).data()) : nullptr;
  Collection* snk = sink ? collections_.get(form_path(program, sink).data()) : nullptr;

  return programs_.to_impl(p)->add_pipeline(src, snk);
}

Collection* PrivateUniverse::add_collection(const char* program, const char* name) {
  return collections_.add(program, name);
}

Status::Code PrivateUniverse::add_source(Pipeline* pipeline, const char* source) {
  std::cout << "getting program\n";
  Program* p = programs_.get_program(pipeline->program);
  if (!p) {
    return Status::NULL_POINTER;
  }

  Collection* src = collections_.get(source);
  return programs_.to_impl(p)->add_source(pipeline, src);
}

Status::Code PrivateUniverse::add_sink(Pipeline* pipeline, const char* sink) {
  Program* p = programs_.get_program(pipeline->program);
  if (!p) {
    return Status::NULL_POINTER;
  }

  Collection* snk = collections_.get(sink);
  return programs_.to_impl(p)->add_source(pipeline, snk);
}

Status::Code PrivateUniverse::share_collection(const char* source, const char* dest) {
  return collections_.share(source, dest);
}

Status::Code PrivateUniverse::copy_collection(const char*, const char*) {
  return Status::OK;
}

}  // namespace radiance
