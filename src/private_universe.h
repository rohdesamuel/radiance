#ifndef PRIVATE_UNIVERSE__H
#define PRIVATE_UNIVERSE__H

#include "radiance.h"
#include "table.h"
#include "stack_memory.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#define LOG_VAR(var) std::cout << #var << " = " << var << std::endl

namespace radiance {

const static char NAMESPACE_DELIMETER = '/';

class PipelineImpl {
 private:
  Pipeline* pipeline_;
  std::vector<Collection*> sources_;
  std::vector<Collection*> sinks_;

 public:
  PipelineImpl(Pipeline* pipeline) : pipeline_(pipeline) {}
  
  void add_source(Collection* source) {
    if (source) {
      if (std::find(sources_.begin(), sources_.end(), source) == sources_.end()) {
        sources_.push_back(source);
      }
    }
  }

  void add_sink(Collection* sink) {
    if (sink) {
      if (std::find(sinks_.begin(), sinks_.end(), sink) == sinks_.end()) {
        sinks_.push_back(sink);
      }
    }
  }

  void run() {
    run_1_to_1();
  }

  void run_1_to_1() {
    
    Collection* source = sources_[0];
    Collection* sink = sinks_[0];

    uint64_t count = source->count(source);

#pragma omp parallel for
    for(uint64_t i = 0; i < count; ++i) {
      thread_local static Stack stack;
      source->copy(
          source->keys.data + source->keys.offset + i * source->keys.size,
          source->values.data + source->values.offset + i * source->values.size,
          i, &stack);
      pipeline_->transform(&stack);
      sink->mutate(sink, (const Mutation*)stack.top());
      stack.clear();
    }
  }

  void run_m_to_n() {
    Stack stack;
    std::unordered_map<uint8_t*, std::vector<uint8_t*>> joined;

    // Naive Hash-Join implementation.
    uint64_t min_count = std::numeric_limits<uint64_t>::max();
    Collection* min_collection = nullptr;
    for (Collection* c : sources_) {
      if (c->count(c) < min_count) {
        min_collection = c;
      }
    }

    {
      uint8_t* keys = min_collection->keys.data + min_collection->keys.offset;
      uint8_t* values = min_collection->values.data + min_collection->values.offset;
      for(uint64_t i = 0; i < min_count; ++i) {
        joined[keys].push_back(values);

        keys += min_collection->keys.size;
        values += min_collection->values.size;
      }
    }

    for (Collection* c : sources_) {
      if (c == min_collection) continue;
      if (c == sources_.back()) continue;

      uint8_t* keys = min_collection->keys.data + min_collection->keys.offset;
      uint8_t* values = min_collection->values.data + min_collection->values.offset;
      for(uint64_t i = 0; i < min_count; ++i) {
        joined[keys].push_back(values);

        keys += c->keys.size;
        values += c->values.size;
      }
    }

    {
      Collection* c = sources_.back();
      uint8_t* keys = c->keys.data + c->keys.offset;
      uint8_t* values = c->values.data + c->values.offset;
      for(uint64_t i = 0; i < min_count; ++i) {
        auto joined_values = joined[keys];
        joined_values.push_back(values);

        if (joined_values.size() == sources_.size()) {
        }

        keys += c->keys.size;
        values += c->values.size;
      }
    }
  }
};

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

  Status::Code remove_pipeline(struct Pipeline* pipeline) {
    return disable_pipeline(pipeline);
  }

  Status::Code enable_pipeline(struct Pipeline* pipeline, ExecutionPolicy policy) {
    disable_pipeline(pipeline);

    if (policy.trigger == Trigger::LOOP) {
      loop_pipelines_.push_back(pipeline);
      std::sort(loop_pipelines_.begin(), loop_pipelines_.end());
    } else if (policy.trigger == Trigger::EVENT) {
      event_pipelines_.insert(pipeline);
    } else {
      return Status::UNKNOWN_TRIGGER_POLICY;
    }

    return Status::OK;
  }

  Status::Code disable_pipeline(struct Pipeline* pipeline) {
    auto found = std::lower_bound(loop_pipelines_.begin(), loop_pipelines_.end(), pipeline);
    if (found != loop_pipelines_.end()) {
      loop_pipelines_.erase(found);
    }

    event_pipelines_.erase(pipeline);
    return Status::OK;
  }

  Status::Code add_source(Pipeline* pipeline, Collection* source) {
    ((PipelineImpl*)(pipeline->self))->add_source(source);
    return add_pipeline_to_collection(pipeline, source, &readers_);
  }

  Status::Code add_sink(Pipeline* pipeline, Collection* sink) {
    ((PipelineImpl*)(pipeline->self))->add_sink(sink);
    return add_pipeline_to_collection(pipeline, sink, &writers_);
  }

  bool contains_pipeline(Pipeline* pipeline) {
    return std::find(pipelines_.begin(), pipelines_.end(), pipeline) != pipelines_.end();
  }

  void run() {
    for(Pipeline* p : loop_pipelines_) {
      ((PipelineImpl*)p->self)->run();
    }
  }

 private:
  Pipeline* new_pipeline(Id id) {
    Pipeline* p = (Pipeline*)malloc(sizeof(Pipeline));
    memset(p, 0, sizeof(Pipeline));
    *(Id*)(&p->id) = id;
    *(Id*)(&p->program) = program_->id;
    p->self = new PipelineImpl(p);
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
  std::vector<Pipeline*> loop_pipelines_;
  std::set<Pipeline*> event_pipelines_;
};

class CollectionRegistry {
 public:
  Collection* add(const char* program, const char* collection) {
    std::string name = std::string(program) + NAMESPACE_DELIMETER + std::string(collection);
    Collection* ret = nullptr;
    if (collections_.find(name) == -1) {
      Handle id = collections_.insert(name, nullptr);
      ret = new_collection(id, collection);
      collections_[id] = ret;
    }
    return ret;
  }

  Status::Code share(const char* source, const char* dest) {
    Handle src = collections_.find(source);
    Handle dst = collections_.find(dest);
    if (src != -1 && dst == -1) {
      collections_.insert(dest, collections_[src]);
      return Status::OK;
    }
    return Status::ALREADY_EXISTS;
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
  Collection* new_collection(Id id, const char* name) {
    Collection* c = (Collection*)malloc(sizeof(Collection));
    memset(c, 0, sizeof(Collection));
    *(Id*)(&c->id) = id;
    *(char**)(&c->name) = (char*)name;
    return c;
  }

  Table<std::string, Collection*> collections_;
};

class ProgramRegistry {
 public:
  Id create_program(const char* program) {
    Id id = programs_.find(program);
    if (id == -1) {
      id = programs_.insert(program, nullptr);
      Program* p = new_program(id, program);
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
    Handle id = programs_.find(program);
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
  Program* new_program(Id id, const char* name) {
    Program* p = (Program*)malloc(sizeof(Program));
    memset(p, 0, sizeof(Program));
    *(Id*)(&p->id) = id;
    *(char**)(&p->name) = (char*)name;
    return p;
  }

  Table<std::string, Program*> programs_;
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

  // Pipeline manipulation.
  struct Pipeline* add_pipeline(const char* program, const char* source, const char* sink);
  struct Pipeline* copy_pipeline(struct Pipeline* pipeline, const char* dest);
  Status::Code remove_pipeline(struct Pipeline* pipeline);

  Status::Code enable_pipeline(struct Pipeline* pipeline, ExecutionPolicy policy);
  Status::Code disable_pipeline(struct Pipeline* pipeline);

  // Collection manipulation.
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

  RunState run_state_;
};

}  // namespace radiance

#endif  // PRIVATE_UNIVERSE__H
