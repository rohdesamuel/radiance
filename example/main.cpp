#include "src/table.h"
#include "src/radiance.h"
#include "stars.h"


int main() {
  typedef radiance::Table<int, int> A;
  typedef radiance::Table<int, int> B;

  A a;
  B b;

  a.insert(0, 1);
  b.insert(0, 3);

  radiance::Multiplexer<A, B> sink(&a, &b);
  radiance::Frame frame;

  {
    A::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = 2;
    frame.result(el);
  }
  {
    B::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = 4;
    frame.result(el);
  }

  radiance::Multiplexer<A, B>::Writer::write(&sink, &frame);

  return 0;
}
