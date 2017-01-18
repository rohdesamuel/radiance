#ifndef STARS__H
#define STARS__H

#include "transformation.h"

#include "src/system.h"

#include <atomic>

#if 0
const static int32_t RADIUS = 10;

using radiance::Frame;

struct Color {
  glm::vec3 c;
};

class Stars {
  typedef radiance::Schema<uint32_t, Color> Rendering;

  struct Components {
    radiance::Handle transformation_handle;
    radiance::Handle rendering_handle;
  };

  struct CreateArgs {
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
    auto* e = frame->result<CreateArgs>();
    Transformations::Mutation m;
    m.el.indexed_by = radiance::IndexedBy::KEY;
    Transformation t;
    t.v = glm::vec3{0, 0, 0};
    t.p = e->p;
    //frame->result<Transformations::Mutation>(std::move(t));
  }
  
  static void update(Frame* frame) {
    auto* e = frame->result<Schema::Element>();
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

#endif

#endif  // STARS__H
