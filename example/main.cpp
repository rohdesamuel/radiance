#include "inc/radiance.h"
#include "inc/table.h"
#include "inc/stack_memory.h"
#include "inc/schema.h"
#include "inc/timer.h"

#include <glm/glm.hpp>
//#include <omp.h>
#include <unordered_map>

typedef std::vector<glm::vec3> Vectors;

struct Transformation {
  glm::vec3 p;
  glm::vec3 v;
};

typedef radiance::Schema<uint32_t, Transformation> Transformations;

int main() {

  uint64_t count = 4096;
  uint64_t iterations = 10000;
  //std::cout << "Max parallelization: " << omp_get_max_threads() << std::endl;
  //std::cout << "Number of threads: " << omp_get_num_threads() << std::endl;
  std::cout << "Number of iterations: " << iterations << std::endl;
  std::cout << "Entity count: " << count << std::endl;

  radiance::Universe uni;
  radiance::init(&uni);

  radiance::create_program("physics"); 
  radiance::Collection* transformations =
      radiance::add_collection("physics", "transformations");

  Transformations::Table* table = new Transformations::Table();
  table->keys.reserve(count);
  table->values.reserve(count);

  for (uint64_t i = 0; i < count; ++i) {
    glm::vec3 p{(float)i, 0, 0};
    glm::vec3 v{1, 0, 0};
    table->insert(i, {p, v});
  }

  radiance::Copy copy = [](const uint8_t*, const uint8_t* value, uint64_t offset, radiance::Stack* stack) {
    radiance::Mutation* mutation = (radiance::Mutation*)stack->alloc(sizeof(radiance::Mutation) + sizeof(Transformations::Element));
    mutation->element = (uint8_t*)(mutation + sizeof(radiance::Mutation));
    mutation->mutate_by = radiance::MutateBy::UPDATE;
    Transformations::Element* el = (Transformations::Element*)(mutation->element);
    el->offset = offset;
    new (&el->value) Transformations::Value( *(Transformations::Value*)(value) );
  };
  radiance::Mutate mutate = [](radiance::Collection* c, const radiance::Mutation* m) {
    Transformations::Table* t = (Transformations::Table*)c->self;
    Transformations::Element* el = (Transformations::Element*)(m->element);
    t->values[el->offset] = std::move(el->value); 
  };

  transformations->self = (uint8_t*)table;
  transformations->copy = copy;
  transformations->mutate = mutate;
  transformations->count = [](radiance::Collection* c) -> uint64_t {
    return ((Transformations::Table*)c->self)->size();
  };

  transformations->keys.data = (uint8_t*)table->keys.data();
  transformations->keys.size = sizeof(Transformations::Key);
  transformations->keys.offset = 0;
  transformations->values.data = (uint8_t*)table->values.data();
  transformations->values.size = sizeof(Transformations::Value);
  transformations->values.offset = 0;

  radiance::Pipeline* pipeline =
      radiance::add_pipeline("physics", "transformations", "transformations");
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
  double total = 0.0;
  double avg = 0.0;
  Timer timer;
  for (uint64_t i = 0; i < iterations; ++i) {
    timer.start();
    radiance::loop();
    timer.stop();
    total += timer.get_elapsed_ns();
  }
  avg = total / iterations;
  std::cout << "elapsed ns: " << total << std::endl;
  std::cout << "avg ns per iteration: " << avg << std::endl;
  std::cout << "avg ns per entity per iteration: " << avg / count << std::endl;
  std::cout << "iteration throughput: " << (1e9 / avg) << std::endl;
  std::cout << "entity throughput: " << count / (avg / 1e9) << std::endl;
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
