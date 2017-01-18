#ifndef TRANSFORMATION__H
#define TRANSFORMATION__H

#include "src/schema.h"

#include <glm/glm.hpp>

struct Transformation {
  glm::vec3 p;
  glm::vec3 v;
};

typedef radiance::Schema<uint32_t, Transformation> Transformations;

#endif  // TRANSFORMATION__H
