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
class Pipeline : public std::function<void(void)> {
public:
  template<class Sys, class... Args>
  Pipeline(Source* source, Sink* sink, Sys system, Args&&... args) : std::function<void(void)>([=]() { run(source, sink, system, args...); }) {};

  template<class Reader = typename Source::Reader, class Writer = typename Sink::Writer, class System, class... Args>
  static void run(Source* source, Sink* sink, System system, Args&&... args) {
    Reader::read(source, System([=](Frame* frame) {
      system(frame);
      Writer::write(sink, frame);
    }), std::forward<Args>(args)...);
  }
};

template<class Source>
class Pipeline<Source, void> : public std::function<void(void)> {
public:
  template<class Sys, class... Args>
  Pipeline(Source* source, Sys system, Args&&... args) : std::function<void(void)>([=]() {run(source, system, args...); }) {};

  template<class Reader = typename Source::Reader, class System, class... Args>
  static void run(Source* source, System system, Args&&... args) {
    Reader::read(source, system, std::forward<Args>(args)...);
  }
};

template<class Sink>
class Pipeline<void, Sink> : public std::function<void(void)> {
public:
  template<class Sys>
  Pipeline(Sink* sink, Sys system) : std::function<void(void)>([=]() { run(sink, system); }) {};

  template<class System, class Writer = typename Sink::Writer>
  static void run(Sink* sink, System system) {
    Writer::write(sink, system());
  }
};

}
#endif
