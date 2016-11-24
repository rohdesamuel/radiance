#ifndef COMPONENT__H
#define COMPONENT__H
#if 0
#include <iostream>
#include <set>

#include "common.h"
#include "radiance.h"
#include "table.h"
#include "vector_math.h"

namespace radiance
{

struct Instance {
  Family family;
  Handle entity;
};

template<class C>
using Instances = Table<Id, C>;

typedef Table<Family, void*> Entities;
typedef Table<Family, class Component*> Components;

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
  typedef std::set<Family> Is;
  typedef std::set<Family> Has;


  /*
  template <class Type_>
  static Component create(const Is& is) {
    Component ret(is);
    Family family = ret.family();
    ret.alloc_ = [family](Component* component, Components* components,
                          Entities* entities, uint64_t count) {
      components->find(family);
      Instances<Type_>* instances = (*entities)[entities->find(family)];
      for (uint64_t i = 0; i < count; ++i) {
        instances->insert(instances->size(), Type_());
      }
    };
    return ret;
  }

  static Component create(const Is& is, const Has& has) {
    Component ret(is, has);
    ret.alloc_ = [=](Component* component, Components* components,
                     Entities* entities, uint64_t count) {
      for(auto family : has) {
        Component* c = (*components)[components->find(family)];
        c->alloc(c, components, entities, count);
      }
    };
    return ret;
  }
  */

  // Composition operations.
  void operator+(const Component& c);
  void operator-(const Component& c);

  static Family family() {
    static Family id = family_counter_++;
    return id;
  }

  const Is& is() {
    return is_;
  }

  const Has& has() {
    return has_;
  }

  static void alloc(Component* component, Components* components,
                    Entities* entities, uint64_t count) {
    component->alloc_(component, components, entities, count);
  }

 private:
  static Family family_counter_;

  Component(const Is& is): is_(is) {}

  Component(const Is& is, const Has& has): is_(is), has_(has) {}

  Is is_;
  Has has_;

  std::function<void(Component*, Components*, Entities*, uint64_t)> alloc_;
};

class ComponentLibrary {
 public:
  Component create();

};

//Component& register_component(std::set<const Component&> is, std::set<const Component&> has) {
//}

/*
template<class C, class... Args>
Instance make_instance(Args&&... args) {
  Component* component = components[components.find(C::family())];

  Instances<C>* table = entities.find(C::family());
  Handle handle = table->insert(C(std::forward<Args>(args)...));
  return {C::family(), handle};
}*/

template<class C>
Instances<C>* get_children() {
  return entities.find(C::family());
}

}
#endif
#endif
