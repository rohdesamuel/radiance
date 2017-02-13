#ifndef STARS__H
#define STARS__H

#include "transformation.h"

#include "src/system.h"

#include <atomic>

const static int32_t RADIUS = 10;

using radiance::Frame;

struct Color {
  glm::vec3 c;
};

typedef radiance::Schema<uint32_t, Color> Rendering;

class Stars {
 public:
  struct Components {
    radiance::Handle transformation_handle;
    radiance::Handle rendering_handle;
  };

  struct CreateArgs {
    glm::vec3 v;
    glm::vec3 p;
  };

  Stars(uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
      float x = rand() % RADIUS;
      float y = rand() % RADIUS;
      float z = rand() % RADIUS;
      Transformations::Value t = {
        {0, 0, 0},
        {x, y, z}
      };

      float r = (float)(rand() % 1000000) / 1000000.0f;
      float g = (float)(rand() % 1000000) / 1000000.0f;
      float b = (float)(rand() % 1000000) / 1000000.0f;
      glm::vec3 color = {r, g, b};
      Rendering::Value data = { color };

      make(i, t, data);
    }
  }

  void make(uint32_t id,
      Transformations::Value transformation,
      Rendering::Value render_data) {
    transformations.insert(id, std::move(transformation));
    rendering.insert(id, std::move(render_data));
  }

  static void create(Frame* frame) {
    auto* args = frame->peek<CreateArgs>();
    Transformations::Mutation tm;
    tm.mutate_by = radiance::MutateBy::INSERT;
    tm.el.key = 0;
    tm.el.value.v = args->v;
    tm.el.value.v = args->p;
    frame->push<Transformations::Mutation>(std::move(tm));

    Rendering::Mutation rm;
    rm.mutate_by = radiance::MutateBy::INSERT;
    rm.el.key = 0;
    rm.el.value.c = glm::vec3{1.0f, 0.0f, 0.0f};
    frame->push<Rendering::Mutation>(std::move(rm));
  }
  
  static void update(Frame* frame) {
    auto* e = frame->peek<Transformations::Element>();
    e->value.p += e->value.v;
  }

  static void draw(Frame*) {
    // draw star here 
  }

  //Components::Table components;
  Transformations::Table transformations;
  Rendering::Table rendering;

 private:
  std::atomic_uint id_;

};

#endif  // STARS__H
