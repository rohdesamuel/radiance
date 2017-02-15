#ifndef SOURCE_MANAGER__H
#define SOURCE_MANAGER__H

#include "common.h"
#include "table.h"
#include "type.h"

namespace radiance {


class SourceManager {
 private:
  Table<Id, void*> sources_;
  Id id_;

 public:
#if 0
  template<class Source_>
  Type<Source_> insert(Source_* source) {
    return Type<Source_>(sources_.insert(id_++, source));
  }

  template<class... Args_>
  void emplace(Id, Args_...) {
  }

  template<class Type_>
  typename Type_::Typedef& operator[](Type_ type) {
    return (typename Type_::Typedef)(sources_[type.id]);
  }
#endif
};

}  // namesapce radiance

#endif  // SOURCE_MANAGER__H
