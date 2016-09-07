/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef PIPELINE__H
#define PIPELINE__H

#include "system.h"

namespace radiance
{

template<class Source, class Sink>
class Pipeline {
public:
  typedef typename Source::Reader Reader;
  typedef typename Sink::Writer Writer;

  template<class Sys>
  Pipeline(Source* source, Sink* sink,
           Reader reader, Writer writer, Sys system) :
           source_(source), sink_(sink), reader_(reader), writer_(writer),
           f_(system) {};

  template<class Sys>
  static void run(Source* source, Sink* sink,
                  Reader reader, Writer writer, Sys system) {
    reader(source, [=](Frame* frame) {
      system(frame);
      writer(sink, frame);
    });
  }

  void operator()(void) {
    reader_(source_, System([=](Frame* frame) {
      f_(frame);
      writer_(sink_, frame);
    }));
  }

private:
  Source* source_;
  Sink* sink_;

  typename Source::Reader reader_;
  typename Sink::Writer writer_;

  std::function<void(Frame*)> f_;
};

template<class Source>
class Pipeline<Source, void> {
public:
  typedef typename Source::Reader Reader;
  typedef void Writer;

  template<class Sys>
  Pipeline(Source* source, Reader reader, Sys system) :
           source_(source), reader_(reader), f_(system) {};

  template<class System>
  static void run(Source* source,
                  Reader reader, System system) {
    reader(source, System([=](Frame* frame) {
      system(frame);
    }));
  }

  void operator()(void) {
    reader_(source_, System([=](Frame* frame) {
      f_(frame);
    }));
  }

private:
  Source* source_;

  typename Source::Reader reader_;

  std::function<void(Frame*)> f_;
};

template<class Sink>
class Pipeline<void, Sink> {
public:
  typedef void Reader;
  typedef typename Sink::Writer Writer;

  template<class Sys>
  Pipeline(Sink* sink, Writer writer, Sys system) :
           sink_(sink), writer_(writer), f_(system) {};

  template<class System>
  static void run(Sink* sink, Writer writer, System system) {
    static thread_local Frame frame;
    writer(sink, system(&frame));
  }

  void operator()(void) {
    static thread_local Frame frame;
    writer_(sink_, f_(&frame));
  }

private:
  Sink* sink_;

  typename Sink::Writer writer_;

  std::function<void(Frame*)> f_;
};

}
#endif
