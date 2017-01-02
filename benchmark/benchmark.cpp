#include <string.h>
#include <iostream>
#include <omp.h>
    
#include "benchmark.h"
#include "utils/timer.h"

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
  ++el->value.x;
  --el->value.y;
}

void Benchmark::transform_2(radiance::Frame* frame) {
  auto* el = frame->result<Positions::Element>();
  ++el->value.x;
  ++el->value.y;
}

void Benchmark::transform_3(radiance::Frame* frame) {
  auto* el = frame->result<Positions::Element>();
  --el->value.x;
  --el->value.y;
}

void Benchmark::run() {
  WindowTimer fps(60);
  uint64_t frame = 0;
  uint32_t max_iterations = 500;
  omp_set_num_threads(4);
  std::cout << "Max parallelization: " << omp_get_max_threads() << std::endl;
  std::cout << "Number of threads: " << omp_get_num_threads() << std::endl;
  std::cout << "Number of iterations: " << max_iterations << std::endl;
  std::cout << "Entity count: " << entity_count_ << std::endl;
  std::cout << "Positions size: " << positions_.values.size() << std::endl;

  Timer t;
  t.start();
  while (frame < max_iterations) {
    fps.start();
    runner_();
    fps.stop();
    fps.step();
    ++frame;
  }
  t.stop();
  float throughput = ((1000 * entity_count_) / (fps.get_avg_elapsed_ns()/1e6));
  std::cout << "Total time = " << t.get_elapsed_ns() / 1e6 << "[ms]\n";
  std::cout << "Frame time = " << fps.get_avg_elapsed_ns()/1e6 << "[ms]\n";
  std::cout << "Throughput = "
    << (int)throughput
    << "[entity/s]\n";
  std::cout << "Per entity = " << fps.get_avg_elapsed_ns() / entity_count_
            << "[ns/entity]\n";
}

int main() {
  uint32_t entity_count = 1000000;
  Benchmark benchmark(entity_count);
  benchmark.run();
  return 0;
}
