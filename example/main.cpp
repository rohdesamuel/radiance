#include "src/table.h"
#include "src/radiance.h"
#include "stars.h"

typedef std::vector<glm::vec3> Vectors;

int main() {
  typedef radiance::Table<int, Vectors> A;
  typedef radiance::Table<int, int> B;

  A a;
  B b;

  a.insert(0, { glm::vec3{0, 1, 2} });
  b.insert(0, 3);

  radiance::Pipeline<A, B> p;
  p.add({
    radiance::Mux<A, B>(&a, &b)
  });


  radiance::Mux<A, B> mux(&a, &b);
  radiance::Frame frame;

  {
    A::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = { glm::vec3{3, 4, 5} };
    frame.result(el);
  }
  {
    B::Element el;
    el.indexed_by = radiance::IndexedBy::OFFSET;
    el.offset = 0;
    el.value = 4;
    frame.result(el);
  }

  mux(&frame);
  frame.pop();


  return 0;
}
