/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef PIPELINE__H
#define PIPELINE__H

#include "system.h"
#include <vector>
#include <omp.h>

namespace radiance
{

class BasePipeline {
 public:
  BasePipeline() {}
  ~BasePipeline() {}

  std::vector<Id> add(std::vector<System> systems) {
    return systems_.push(systems);
  }

  Id add(System& system) {
    return systems_.push(system);
  }

  void erase(Id id) {
    systems_.erase(id);
  }

protected:
  SystemExecutor systems_;
};

template<typename Source_, typename Sink_>
class Pipeline_ : public BasePipeline {
 public:
  template<typename Reader_, typename Writer_>
  Pipeline_(Source_* source, Sink_* sink,
            Reader_ reader, Writer_ writer):
      source_(source),
      sink_(sink),
      reader_(reader),
      writer_(writer) {
    frames_ = new Frame [1];
  }

  ~Pipeline_() {
    delete[] frames_;
  }

  void operator()(void) {

    reader_(source_, systems_);
  }

 private:
  System reader_;
  System writer_;
  Source_* source_;
  Sink_* sink_;
  Frame* frames_;
};

template<typename Source_, typename Sink_>
class Pipeline: public BasePipeline {
public:
  typedef Source_ Source;
  typedef Sink_ Sink;
  typedef typename Source::Reader Reader;
  typedef typename Sink::Writer Writer;

  Pipeline() {};

  Pipeline(Source* source, Sink* sink,
           Reader reader, Writer writer) :
           reader_(reader), writer_(writer), source_(source), sink_(sink) {
    sink_ = sink;
    // Why do we need to directly capture "sink" instead of "sink_"?
    systems_ = SystemExecutor([writer, sink](Frame* frame) {
      writer(sink, frame);
    });
  }

  void operator()(void) {
    reader_(source_, systems_);
  }

private:
  Reader reader_;
  Writer writer_;
  Source* source_;
  Sink* sink_;
};

template<typename Source_>
class Pipeline<Source_, void>: public BasePipeline {
public:
  typedef Source_ Source;
  typedef void Sink;
  typedef typename Source::Reader Reader;
  typedef void Writer;

  Pipeline() {};

  Pipeline(Source* source, Reader reader) :
           reader_(reader), source_(source) {
  }

  void operator()(void) {
    reader_(source_, systems_);
  }

private:
  Reader reader_;
  Source* source_;
};

template<typename Sink_>
class Pipeline<void, Sink_>: public BasePipeline {
public:
  typedef void Source;
  typedef Sink_ Sink;
  typedef void Reader;
  typedef typename Sink::Writer Writer;

  Pipeline() {};

  Pipeline(Sink* sink, Writer writer) :
           writer_(writer), sink_(sink) {
    systems_ = SystemExecutor([this](Frame* frame) {
      writer_(sink_, frame);
    });
  };

  void operator()(void) {
    static thread_local Frame frame;
    systems_(&frame);
    writer_(sink_, &frame);
  }

private:
  Writer writer_;
  Sink* sink_;
};

}  // namespace radiance

#endif
