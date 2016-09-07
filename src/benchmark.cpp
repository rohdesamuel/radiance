#include <string.h>
#include <iostream>
    
#include "benchmark.h"
#include "timer.h"

Benchmark::Benchmark(uint32_t entity_count): entity_count_(entity_count) {
  for (uint32_t i = 0; i < entity_count_; ++i) {
    positions_.insert(i, { (float)(rand() % 1000), (float)(rand() % 1000)});
  }

  benchmark_system_ =
    radiance::System(transform_1) *
    radiance::System(transform_2) *
    radiance::System(transform_3);
}

void Benchmark::transform_1(radiance::Frame* frame) {
  auto* el = frame->result<Positions::Element>();
  ++el->component.x;
  --el->component.y;
}

void Benchmark::transform_2(radiance::Frame* frame) {
  auto* el = frame->result<Positions::Element>();
  ++el->component.x;
  ++el->component.y;
}

void Benchmark::transform_3(radiance::Frame* frame) {
  auto* el = frame->result<Positions::Element>();
  --el->component.x;
  --el->component.y;
}

void Benchmark::run() {
  WindowTimer fps(60);
  uint64_t frame = 0;
  uint32_t max_iterations = 10000;
  while (frame < max_iterations) {
    fps.start();
    radiance::Pipeline<Positions, Positions>::run(
        &positions_,
        &positions_,
        Positions::Reader(radiance::IndexedBy::OFFSET),
        Positions::Writer{},
        benchmark_system_
    );
    fps.stop();
    fps.step();

    ++frame;
  }
  for (auto& c : positions_.components) {
    std::cout << "(" << c.x << ", " << c.y << ")\r";
  }
  std::cout << "Throughput = "
    << (int)((1000 * entity_count_) / (fps.get_avg_elapsed_ns()/1e6))
    << "[entity/s]\n";
}
