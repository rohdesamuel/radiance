#include "src/radiance.h"
#include "src/table.h"
#include "src/schema.h"

#include <glm/glm.hpp>

#include <unordered_map>

typedef std::vector<glm::vec3> Vectors;

namespace radiance {

#if 0
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
#endif

}  // namespace radiance

struct Transformation {
  glm::vec3 p;
  glm::vec3 v;
};

typedef radiance::Schema<uint32_t, Transformation> Transformations;

int main() {
  radiance::Universe uni;
  radiance::init(&uni);

  radiance::create_program("physics");
  radiance::Collection* transformations =
      radiance::add_collection("physics", "transformations");

  Transformations::Table* table = new Transformations::Table();
  radiance::Mutate mutate = [](radiance::Collection* c, const radiance::Mutation* m) {
    Transformations::Table* t = (Transformations::Table*)c->self;
    Transformations::Element* el = (Transformations::Element*)(m->element);
    t->insert(el->key, el->value);
  };
  radiance::Iterate iterate = [](radiance::Collection* c, radiance::Stack* s, uint8_t* state) -> uint8_t* {
    thread_local static uint64_t index = 0;
    uint64_t* ret = &index;
    Transformations::Table* t = (Transformations::Table*)c->self;

    if (state == nullptr) {
      index = 0;
    } else {
      ++index;
      if (index >= t->size()) {
        return (uint8_t*)nullptr;
      }
    }
    std::tuple<Transformations::Key, Transformations::Value> el{t->key(index), t->value(index)};
    *((std::tuple<Transformations::Key, Transformations::Value>*)(s->alloc(sizeof(el)))) = el;

    return (uint8_t*)ret;
  };

  transformations->self = (uint8_t*)table;
  transformations->mutate = mutate;
  transformations->iterate = iterate;

  radiance::Pipeline* pipeline =
      radiance::add_pipeline("physics", "transformations", "transformations");
  pipeline->select = nullptr;

  radiance::create_program("render");
  radiance::add_collection("render", "meshes");
  radiance::share_collection("physics/transformations", "render/transformations");

  radiance::Pipeline* render_pipeline =
      radiance::add_pipeline("render", "transformations", nullptr);
  radiance::add_source(render_pipeline, "render/meshes");
  render_pipeline->select = nullptr;
  render_pipeline->transform = [](radiance::Stack*){};

  radiance::start();
  radiance::loop();
  radiance::stop();

  /*
  typedef radiance::Schema<int, Vectors> A;
  typedef radiance::Schema<int, int> B;

  A::Table a;
  B::Table b;

  radiance::SourceManager sources;

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
*/

  return 0;
}
