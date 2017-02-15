#ifndef TYPE__H
#define TYPE__H

#include "common.h"
#include "radiance.h"

#include <string>

namespace radiance {

template<class Type_>
class Type {
 public:
  typedef Type_ Typedef;

  Type(Id id): id(id) {}

  explicit operator Type<void*>() {
    return Type{id};
  }

  const Id id;
};

template<class Type_>
class Var {
 public:
  typedef typename Type_::Typedef Typedef;
  Var() {}

  Type_ type;

  Typedef operator->() {
    return &data_;
  }

 private:
  Typedef data_;
};

}  // namespace radiance

#endif  // TYPE__H
