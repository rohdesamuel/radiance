#include <string.h>
#include <iostream>
    
#include "benchmark.h"
#include "timer.h"

Benchmark::Benchmark(uint32_t entity_count): entity_count_(entity_count) {
  for (uint32_t i = 0; i < entity_count_; ++i) {
    positions_.insert(i, { (float)(rand() % 1000), (float)(rand() % 1000)});
  }

  runner_ = radiance::Pipeline<Positions, Positions>(
      &positions_,
      &positions_,
      Positions::Reader(radiance::IndexedBy::OFFSET),
      Positions::Writer{});
  runner_.add({
    radiance::System(transform_1),
    radiance::System(transform_2),
    radiance::System(transform_3)
  });
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
  uint32_t max_iterations = 3;
  std::cout << "Number of iterations: " << max_iterations << std::endl;
  std::cout << "Entity count: " << entity_count_ << std::endl;
  std::cout << "Positions size: " << positions_.components.size() << std::endl;

  for (auto& c : positions_.components) {
    std::cout << "(" << c.x << ", " << c.y << ")\n";
  }
  while (frame < max_iterations) {
    fps.start();
    runner_();
    fps.stop();
    fps.step();
    ++frame;
  }
  for (auto& c : positions_.components) {
    std::cout << "(" << c.x << ", " << c.y << ")\n";
  }
  std::cout << "Frame time: " << fps.get_avg_elapsed_ns()/1e6 << "[ms]\n";
  std::cout << "Throughput = "
    << (int)((1000 * entity_count_) / (fps.get_avg_elapsed_ns()/1e6))
    << "[entity/s]\n";
}
