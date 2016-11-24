#ifndef OPERATORS__H
#define OPERATORS__H

#include "common.h"
#include "system.h"

namespace radiance
{

template<class Source_>
class ReaderInterface {
public:
  typedef Source_ Source;

  virtual bool is_thread_safe() = 0;

  template<class System>
  void operator()(Source* source, System system) const {
    read(source, system);
  };
};

template<class Sink_>
class WriterInterface {
public:
  typedef Sink_ Sink;

  virtual bool is_thread_safe() = 0;

  void operator()(Sink* sink, Frame* frame) {
    write(sink, frame);
  };
};

}  // namespace radiance
#endif  // OPERATORS__H
