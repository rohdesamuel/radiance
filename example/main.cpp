#include "src/radiance.h"
#include "src/table.h"
#include "src/stack_memory.h"
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

  std::cout << "Program id: " << radiance::create_program("physics") << std::endl; 
  radiance::Collection* transformations =
      radiance::add_collection("physics", "transformations");

  Transformations::Table* table = new Transformations::Table();
  radiance::Copy copy = [](const uint8_t* key, const uint8_t* value, radiance::Stack* stack) {
    radiance::Mutation* mutation = (radiance::Mutation*)stack->alloc(sizeof(radiance::Mutation) + sizeof(Transformations::Element));
    mutation->element = (uint8_t*)(mutation + sizeof(radiance::Mutation));
    mutation->mutate_by = radiance::MutateBy::UPDATE;
    Transformations::Element* el = (Transformations::Element*)(mutation->element);
    new (&el->key) Transformations::Key( *(Transformations::Key*)(key) );
    new (&el->value) Transformations::Value( *(Transformations::Value*)(value) );
  };
  radiance::Mutate mutate = [](radiance::Collection* c, const radiance::Mutation* m) {
    Transformations::Table* t = (Transformations::Table*)c->self;
    Transformations::Element* el = (Transformations::Element*)(m->element);
    (*t)[t->find(el->key)] = el->value;
  };

  transformations->self = (uint8_t*)table;
  transformations->copy = copy;
  transformations->mutate = mutate;
  transformations->keys.data = (uint8_t*)table->keys.data();
  transformations->keys.size = sizeof(Transformations::Key);
  transformations->keys.offset = 0;
  transformations->values.data = (uint8_t*)table->values.data();
  transformations->values.size = sizeof(Transformations::Value);
  transformations->values.offset = 0;

  radiance::Pipeline* pipeline =
      radiance::add_pipeline("physics", "transformations", "transformations");
  std::cout << "Created pipeline: " << pipeline << std::endl;
  pipeline->select = nullptr;
  pipeline->transform = [](radiance::Stack* s) {
    Transformations::Element* el = (Transformations::Element*)((radiance::Mutation*)(s->top()))->element;
    el->value.p += el->value.v;
  };

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
