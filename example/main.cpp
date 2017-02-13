#include "src/table.h"
#include "src/radiance.h"
#include "stars.h"

#include <unordered_map>

typedef std::vector<glm::vec3> Vectors;

namespace radiance {

struct Collection {
  uint8_t* keys;
  uint8_t* values;
  uint64_t count;
  size_t key_size;
  size_t value_size;
};

void retrieve(Collection* collection, Stack* stack, uint64_t pos) {
  uint8_t* mem = (uint8_t*)stack->alloc(collection->key_size);
  memcpy(collection->keys + pos * collection->key_size, mem, collection->key_size);

  mem = (uint8_t*)stack->alloc(collection->value_size);
  memcpy(collection->values + pos * collection->value_size, mem, collection->value_size);
}

Collection run_pipeline(struct _Pipeline* pipeline) {
  pipeline->read(pipeline->source, &pipeline->stack);
  return Collection{};
}


template <class... Sources_>
class Entity {
 public:
  class Reader {
   public:
    void operator()(Entity* entity, const SystemExecutor& system) const {
      read(entity, system, std::index_sequence_for<Sources_*...>());
    }

   private:
    template<class Source_>
    void read(Source_* source, Handle handle, Frame* frame) const {
      frame->push((*source)[handle]);
    }

    template<std::size_t... Index_>
    void read(Entity* entity, const SystemExecutor& system,
        std::index_sequence<Index_...>) const {
      for(const auto& component_source : entity->components.values) {
        static thread_local Frame frame;
        using swallow = int[];
        (void)swallow{0, (void(
              read(std::get<Index_>(entity->sources_), component_source[Index_], &frame))
        , 0)...};
        system(&frame);
        frame.clear();
      }
    };
  };

  Entity(Sources_*... sources) {
    sources_ = std::tuple<Sources_*...>{sources...};
  }

  Table<uint64_t, std::vector<Handle>> components;

 private:
  std::tuple<Sources_*...> sources_;
};

template<class... Types_>
class Join {
 public:
  class Reader {
   public:
    void operator()(Join* join, const SystemExecutor& system) const {
      read(join, system, std::index_sequence_for<Types_...>());
    }

   private:
    template<class Type_>
    void read(Type_* type, Handle handle, Frame* frame) const {
      typename Type_::Typedef* source = (*universe()->source_manager)[*type];
      frame->push((*source)[handle]);
    }

    template<std::size_t... Index_>
    void read(Join* join, const SystemExecutor& system,
        std::index_sequence<Index_...>) const {
      for(const auto& component_source : join->components.values) {
        static thread_local Frame frame;
        using swallow = int[];
        (void)swallow{0, (void(
              read(std::get<Index_>(join->types_), component_source[Index_], &frame))
        , 0)...};
        system(&frame);
        frame.clear();
      }
    };
  };

  Join(Types_... types) {
    types_ = std::tuple<Types_...>{types...};
  }

  Table<uint64_t, std::vector<Handle>> components;

 private:
  std::tuple<Types_...> types_;
};

template <class Source_>
class Iterator {
 public:
  struct Options {
  };

  Iterator(Options options): options_(options) {}

  void operator()(Source_* source, SystemExecutor& system) const {
    for (int64_t i = 0; i < (int64_t)source->size(); ++i) {
      thread_local static Frame frame;
      frame.clear();
      frame.peek(reader(source, i));
      system(&frame);
    }
  }

 private:
  Options options_;
};

}  // namespace radiance

int main() {
  using radiance::Pipeline;
  using radiance::Mux;
  using radiance::Type;

  radiance::Universe uni;
  radiance::start(&uni);

  typedef radiance::Schema<int, Vectors> A;
  typedef radiance::Schema<int, int> B;

  A::Table a;
  B::Table b;

  radiance::SourceManager sources;
  Transformations::Table transformations;

  Type<Transformations::Table> type_a = sources.insert(&transformations);
  typename decltype(type_a)::Typedef ptr_a = sources[type_a];

  a.insert(0, { glm::vec3{0, 1, 2} });
  b.insert(0, 3);

  radiance::System start([](radiance::Frame*) { std::cout << "start\n"; });
  radiance::System u([](radiance::Frame*) { std::cout << "u\n"; });
  radiance::System v([](radiance::Frame*) { std::cout << "v\n"; });
  radiance::System w([](radiance::Frame*) { std::cout << "w\n"; });
  radiance::System x([](radiance::Frame*) { std::cout << "x\n"; });
  radiance::System y([](radiance::Frame*) { std::cout << "y\n"; });
  radiance::System z([](radiance::Frame*) { std::cout << "z\n"; });
  radiance::System end([](radiance::Frame*) { std::cout << "end\n"; });

  typedef radiance::Entity<Transformations::Table, Rendering::Table> Stars;
  Rendering::Table rendering;

  Stars stars(&transformations, &rendering);
  stars.components.insert(0,
      {transformations.insert(0, {}),
       rendering.insert(0, {})});
  stars.components.insert(1,
      {transformations.insert(1, {}),
       rendering.insert(1, {})});

  Stars::Reader r;
  radiance::SystemExecutor executor;
  executor.push({ x, y, z });
  r(&stars, executor);


  Mux<A::Table, B::Table> mux(&a, &b);
  radiance::Frame frame;

  {
    A::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = { glm::vec3{3, 4, 5} };
    frame.push(el);
  }
  {
    B::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = 4;
    frame.push(el);
  }

  frame.pop();


  return 0;
}
