#include <iostream>

#include "radiance.h"
#include "vector_math.h"

namespace radiance
{

struct Instance {
  Id family;
  Handle entity;
};

// Give me all the children components of component C.
// Give me all the entities of component C.
// Give me all the children entities of component C.
// Give me all the attributes of component C.
// Give me the system for attribute A of component C.
//
// Create component C and all its children.
// Destroy component C and all its children.
//
// Run all systems on all components of type C.
//
// Components are only composable.
// Components cannot form a hierarchy.
class Component {
public:
  // Composition operations.
  Component operator+(Component c);
  Component operator-(Component c);

  // Definition operations.
  Component define_as(std::vector<Component>&&);

  Id family;
};

}
