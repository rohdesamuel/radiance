#include <iostream>

#include "radiance.h"
#include "vector_math.h"

namespace radiance
{

struct Instance {
  Id family;
  Handle entity;
};

// Give me all the entities of this type.
// Give me all the children entities of this type.
// Give me all the attributes for this type.
// Give me the system for an attribute.
struct Component {
  typedef Id Attribute;
  std::vector<Attribute> attributes;
  std::vector<Component> components;
  Id family;
  Handle entity;
};

}
