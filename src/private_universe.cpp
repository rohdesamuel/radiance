#include "private_universe.h"

#include <algorithm>

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
  Id ret = -1;
  if (programs_.find(name) == programs_.end()) {
    ret = programs_.size();
    programs_[name] = new Program();
  }
  return ret;
}

Pipeline* PrivateUniverse::add_pipeline(const char* program, const char* source, const char* sink) {
  
  return nullptr;
}

Collection* PrivateUniverse::add_collection(const char* program, const char* name) {
  return nullptr;
}

Status::Code PrivateUniverse::add_source(Pipeline* pipeline, const char* source) {
  return Status::OK;
}

Status::Code PrivateUniverse::add_sink(Pipeline* pipeline, const char* sink) {
  return Status::OK;
}

Status::Code PrivateUniverse::share_collection(const char* source, const char* dest) {
  return Status::OK;
}

Status::Code PrivateUniverse::copy_collection(const char* source, const char* dest) {
  return Status::OK;
}

}  // namespace radiance
