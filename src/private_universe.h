#ifndef PRIVATE_UNIVERSE__H
#define PRIVATE_UNIVERSE__H

#include "radiance.h"
#include "table.h"
#include "stack_memory.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>

namespace radiance {

const static char NAMESPACE_DELIMETER = '/';

class ProgramImpl {
 private:
   typedef Table<Collection*, std::set<Pipeline*>> Mutators;
 public:
  ProgramImpl(Program* program) : program_(program) {}

  Pipeline* add_pipeline(Collection* source, Collection* sink) {
    Pipeline* pipeline = new_pipeline(pipelines_.size());
    pipelines_.push_back(pipeline);

    add_source(pipeline, source);
    add_sink(pipeline, sink);

    return pipeline;
  }

  Status::Code add_source(Pipeline* pipeline, Collection* source) {
    return add_pipeline_to_collection(pipeline, source, &readers_);
  }

  Status::Code add_sink(Pipeline* pipeline, Collection* sink) {
    return add_pipeline_to_collection(pipeline, sink, &writers_);
  }

  bool contains_pipeline(Pipeline* pipeline) {
    return std::find(pipelines_.begin(), pipelines_.end(), pipeline) != pipelines_.end();
  }

 private:
  Pipeline* new_pipeline(Id id) {
    Pipeline* p = (Pipeline*)malloc(sizeof(Pipeline));
    memset(p, 0, sizeof(Pipeline));
    *(Id*)(p->program) = program_->id;
    *(Id*)(p->id) = id;
    return p;
  }

  Status::Code add_pipeline_to_collection(
      Pipeline* pipeline, Collection* collection, Mutators* table) {
    if (!pipeline || !collection || !table) {
      return Status::NULL_POINTER;
    }

    if (!contains_pipeline(pipeline)) {
      return Status::DOES_NOT_EXIST;
    }

    Handle handle = table->find(collection);
    if (handle == -1) {
      handle = table->insert(collection, {});
    }

    std::set<Pipeline*>& pipelines = (*table)[handle];

    if (pipelines.find(pipeline) == pipelines.end()) {
      pipelines.insert(pipeline);
    } else {
      return Status::ALREADY_EXISTS;
    }

    return Status::OK;
  }

  Program* program_;
  Mutators writers_;
  Mutators readers_;
  std::vector<Pipeline*> pipelines_;
};

class CollectionRegistry {
 public:
  Collection* add(const char* program, const char* collection) {
    std::string name = std::string(program) + NAMESPACE_DELIMETER + std::string(collection);
    Collection* ret = nullptr;
    if (collections_.find(name) == -1) {
      Handle id = collections_.insert(name, nullptr);
      ret = new_collection(id);
      collections_[id] = ret;
    }
    return ret;
  }

  Collection* get(const char* name) {
    Collection* ret = nullptr;
    Handle id;
    if ((id = collections_.find(name)) != -1) {
      ret = collections_[id];
    }
    return ret;
  }

  Collection* get(const char* program, const char* collection) {
    std::string name = std::string(program) + NAMESPACE_DELIMETER + std::string(collection);
    return get(name.data());
  }

 private:
  Collection* new_collection(Id id) {
    Collection* c = (Collection*)malloc(sizeof(Collection));
    memset(c, 0, sizeof(Collection));
    *(Id*)(c->id) = id;
    return c;
  }

  Table<std::string, Collection*> collections_;
};

class ProgramRegistry {
 public:
  Id create_program(const char* program) {
    std::string name{program};
    Id id = programs_.find(name);
    if (id != -1) {
      id = programs_.insert(name, nullptr);
      Program* p = new_program(id);
      p->self = new ProgramImpl{p};
      programs_[id] = p;
    }
    return id;
  }

  Status::Code add_source(Pipeline* pipeline, Collection* collection) {
    Program* p = programs_[pipeline->program];
    return to_impl(p)->add_source(pipeline, collection);
  }

  Status::Code add_sink(Pipeline* pipeline, Collection* collection) {
    Program* p = programs_[pipeline->program];
    return to_impl(p)->add_sink(pipeline, collection);
  }

  Program* get_program(const char* program) {
    Handle id = programs_.find({program});
    if (id == -1) {
      return nullptr;
    }
    return programs_[id];
  }

  Program* get_program(Id id) {
    return programs_[id];
  }

  inline ProgramImpl* to_impl(Program* p) {
    return (ProgramImpl*)(p->self);
  }
 private:
  Program* new_program(Id id) {
    Program* p = (Program*)malloc(sizeof(Program));
    memset(p, 0, sizeof(Program));
    *(Id*)(p->id) = id;
    return p;
  }

  Table<std::string, Program*> programs_;
};

class PipelineImpl {
 private:
  Pipeline* pipeline_;
  std::vector<Collection*> sources_;
  std::vector<Collection*> sinks_;

 public:
  PipelineImpl(Pipeline* pipeline);
  
  void add_source(Collection* source);
  void add_sink(Collection* sink);

  void run_1_to_1() {
    Stack stack;
    Stack state_stack;
    
    Collection* source = sources_[0];
    Collection* sink = sinks_[0];

    uint8_t* state = source->iterate(source, &stack, nullptr);
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
      state = source->iterate(source, &stack, state);
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

  CollectionRegistry collections_;
  ProgramRegistry programs_;
  std::unordered_map<Program*, std::vector<Pipeline>> pipelines_;

  RunState run_state_;
};

}  // namespace radiance

#endif  // PRIVATE_UNIVERSE__H
