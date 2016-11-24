#ifndef BENCHMARK__H
#define BENCHMARK__H

#include "radiance.h"

class Benchmark {
public:
  struct Position {
    float x, y;
  };

  Benchmark(uint32_t entity_count);

  typedef radiance::Schema<uint32_t, Position> PositionSchema;
  typedef PositionSchema::Table Positions;

  void run();

  static void transform_1(radiance::Frame* frame);
  static void transform_2(radiance::Frame* frame);
  static void transform_3(radiance::Frame* frame);

private:
  radiance::Pipeline<Positions, Positions> runner_;
  Positions positions_;
  radiance::SystemList benchmark_system_;
  uint32_t entity_count_ = 0;
};

#endif
