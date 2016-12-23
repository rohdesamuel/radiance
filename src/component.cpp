#include "component.h"
namespace radiance {
  Id CounterBase::counter_ = 0;
}
#if 0
namespace radiance {

Family Component::family_counter_ = 0;

void Component::operator+(const Component& c) {
  this->has_.insert(c.family());
}

void Component::operator-(const Component& c) {
  this->has_.erase(c.family());
}

}
#endif
