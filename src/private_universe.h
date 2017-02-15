#ifndef PRIVATE_UNIVERSE__H
#define PRIVATE_UNIVERSE__H

#include "radiance.h"
#include "stack_memory.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace radiance {

class CollectionRegistry {
 public:
  Collection* add_collection(const char* program, const char* collection) {
    std::string name = std::string(program) + "/" + std::string(collection);
    Collection* ret = nullptr;
    if (collections_.find(name) == collections_.end()) {
      collections_[name] = ret = new_collection(collections_.size());
    }
    return ret;
  }

 private:
  Collection* new_collection(Id id) {
    Collection* c = (Collection*)malloc(sizeof(Collection));
    memset(c, 0, sizeof(Collection));
    *(Id*)(c->id) = id;
    return c;
  }

  std::unordered_map<std::string, Collection*> collections_;
};

class PipelineImpl {
 private:
  Pipeline* pipeline_;
  std::vector<Collection*> sources_;
  std::vector<Collection*> sinks_;

  size_t source_size_;
  size_t sink_size_;
 public:
  PipelineImpl(Pipeline* pipeline);
  
  void add_source(Collection* source);
  void add_sink(Collection* sink);

  void run_1_to_1() {
    Stack stack;
    Collection* source = sources_[0];
    Collection* sink = sinks_[0];

    void* state = source->iterate(source, &stack, nullptr);
    while(state) {
      if (pipeline_->select) {
        if (pipeline_->select(1, &stack)) {
          pipeline_->transform(&stack);
          sink->mutate(sink, (Mutation*)stack.top());
        }
      } else {
        pipeline_->transform(&stack);
        sink->mutate(sink, (const Mutation*)stack.top());
      }
      stack.clear();
      state = source->iterate(source, &stack, nullptr);
    }
  }

  void run_m_to_n() {
    Stack stack;
    std::vector<uint8_t*> states;
    bool should_continue = true;
    for (Collection* c : sources_) {
      uint8_t* state = c->iterate(c, &stack, nullptr);
      states.push_back(state);
      should_continue &= state == nullptr;
    }

    if (pipeline_->select) {
      uint8_t count = (uint8_t)sources_.size();
      while(should_continue) {
        if (pipeline_->select(count, &stack)) {
          pipeline_->transform(&stack);
          for(Collection* c : sinks_) {
            c->mutate(c, (const Mutation*)stack.top());
          }
        }
        stack.clear();
        for (Collection* c : sources_) {
          uint8_t* state = c->iterate(c, &stack, nullptr);
          states.push_back(state);
          should_continue &= state == nullptr;
        }
      }
    } else {
      while(should_continue) {
        pipeline_->transform(&stack);
        for(Collection* c : sinks_) {
          c->mutate(c, (const Mutation*)stack.top());
        }
        stack.clear();
        for (Collection* c : sources_) {
          uint8_t* state = c->iterate(c, &stack, nullptr);
          states.push_back(state);
          should_continue &= state == nullptr;
        }
      }
    }
  }
};

class ProgramImpl {
 public:
  Pipeline* add_pipeline(Collection* source, Collection* sink);

  Status::Code add_source(Pipeline* pipeline, Collection* source);
  Status::Code add_sink(Pipeline* pipeline, Collection* sink);
};

class PrivateUniverse {
 public:
  enum class RunState {
    ERROR = -1,
    UNKNOWN = 0,
    INITIALIZED,
    STARTED,
    RUNNING,
    STOPPED,
  };

  PrivateUniverse();
  ~PrivateUniverse();

  Status::Code init();
  Status::Code start();
  Status::Code stop();
  Status::Code loop();

  Id create_program(const char* name);
  Pipeline* add_pipeline(const char* program, const char* source, const char* sink);
  Collection* add_collection(const char* program, const char* collection);

  Status::Code add_source(Pipeline*, const char* collection);
  Status::Code add_sink(Pipeline*, const char* collection);

  Status::Code share_collection(const char* source, const char* dest);
  Status::Code copy_collection(const char* source, const char* dest);

 private:
  Status::Code transition(RunState allowed, RunState next);
  Status::Code transition(std::vector<RunState>&& allowed, RunState next);

  CollectionRegistry collections;
  std::unordered_map<const char*, Program*> programs_;
  std::unordered_map<Program*, std::vector<Pipeline>> pipelines_;

  RunState run_state_;
};

}  // namespace radiance

#endif  // PRIVATE_UNIVERSE__H
