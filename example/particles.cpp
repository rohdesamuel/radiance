#include "inc/radiance.h"
#include "inc/stack_memory.h"

#include "particles.h"

radiance::Collection* add_particle_collection(const char* collection, uint64_t particle_count) {
  radiance::Collection* particles =
      radiance::add_collection(kMainProgram, collection);

  Particles::Table* table = new Particles::Table();
  table->keys.reserve(particle_count);
  table->values.reserve(particle_count);

  for (uint64_t i = 0; i < particle_count; ++i) {
    glm::vec3 p{
      2 * (((float)(rand() % 1000) / 1000.0f) - 0.5f),
      2 * (((float)(rand() % 1000) / 1000.0f) - 0.5f),
      2 * (((float)(rand() % 1000) / 1000.0f) - 0.5f)
    };

    glm::vec3 v{
      ((float)(rand() % 1000) / 10000.0f) - 0.05f,
      ((float)(rand() % 1000) / 10000.0f) - 0.05f,
      ((float)(rand() % 1000) / 10000.0f) - 0.05f
    };

    table->insert(i, {p, v});
  }

  radiance::Copy copy =
      [](const uint8_t*, const uint8_t* value, uint64_t offset,
         radiance::Stack* stack) {
        radiance::Mutation* mutation = (radiance::Mutation*)stack->alloc(
            sizeof(radiance::Mutation) + sizeof(Particles::Element));
        mutation->element = (uint8_t*)(mutation + sizeof(radiance::Mutation));
        mutation->mutate_by = radiance::MutateBy::UPDATE;
        Particles::Element* el =
            (Particles::Element*)(mutation->element);
        el->offset = offset;
        new (&el->value) Particles::Value(
            *(Particles::Value*)(value) );
      };

  radiance::Mutate mutate =
      [](radiance::Collection* c, const radiance::Mutation* m) {
        Particles::Table* t = (Particles::Table*)c->collection;
        Particles::Element* el = (Particles::Element*)(m->element);
        t->values[el->offset] = std::move(el->value); 
      };

  particles->collection = (uint8_t*)table;
  particles->copy = copy;
  particles->mutate = mutate;
  particles->count = [](radiance::Collection* c) -> uint64_t {
    return ((Particles::Table*)c->collection)->size();
  };

  particles->keys.data = (uint8_t*)table->keys.data();
  particles->keys.size = sizeof(Particles::Key);
  particles->keys.offset = 0;
  particles->values.data = (uint8_t*)table->values.data();
  particles->values.size = sizeof(Particles::Value);
  particles->values.offset = 0;
  return particles;
}

void add_particle_pipeline(const char* collection) {
  radiance::Pipeline* pipeline =
    radiance::add_pipeline(kMainProgram, collection, collection);
  pipeline->select = nullptr;
  pipeline->transform = [](radiance::Stack* s) {
    Particles::Element* el =
        (Particles::Element*)((radiance::Mutation*)(s->top()))->element;
    el->value.p += el->value.v;
    if (el->value.p.x >  1.0f) { el->value.p.x =  1.0f; el->value.v.x *= -1; }
    if (el->value.p.x < -1.0f) { el->value.p.x = -1.0f; el->value.v.x *= -1; }
    if (el->value.p.y >  1.0f) { el->value.p.y =  1.0f; el->value.v.y *= -1; }
    if (el->value.p.y < -1.0f) { el->value.p.y = -1.0f; el->value.v.y *= -1; }
  };

  radiance::ExecutionPolicy policy;
  policy.priority = radiance::MAX_PRIORITY;
  policy.trigger = radiance::Trigger::LOOP;
  enable_pipeline(pipeline, policy);
}

