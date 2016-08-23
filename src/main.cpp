/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#include <cstdint>

#include <iostream>
#include <sstream>

#include <string.h>
#include <time.h>

#include "radiance.h"

#include "timer.h"
#include "vector_math.h"

#if 0
using namespace radiance;

struct Transformation {
  Vec3 p;
  Vec3 v;
};
typedef ::boost::container::vector<std::tuple<Handle, float>> Spring;

typedef Schema<Handle, Spring> SpringsSchema;
typedef Schema<Entity, Transformation> TransformationSchema;
typedef SpringsSchema::Table Springs;
typedef TransformationSchema::Table Transformations;

int main() {
  const int MAX_HEIGHT = 640 / 8;
  const int MAX_WIDTH = 640 / 8;

  Springs springs;
  Transformations transformations;
  
  TransformationSchema::MutationQueue transformations_queue;

  uint64_t count = 1000;

  for (uint64_t i = 0; i < count; ++i) {
    Vec3 p = {
      (float)(rand() % MAX_WIDTH), (float)(rand() % MAX_HEIGHT),
      0
    };
    Vec3 v;
    v.x = (((float)(rand() % 10000)) - 5000.0f) / 50000.0f;
    v.y = (((float)(rand() % 10000)) - 5000.0f) / 50000.0f;
    transformations_queue.emplace<MutateBy::INSERT, IndexedBy::OFFSET>(
      (int64_t)i, { p, v });
  }
  transformations_queue.flush(&transformations);
  srand((uint32_t)time(NULL));
  for (uint64_t i = 0; i < count; ++i) {
    for (uint64_t j = 0; j < count; ++j) {
      Springs::Component springs_list;
      springs_list.push_back(std::tuple<Handle, float>{ j, 0.0000001 });
      springs.insert(i, std::move(springs_list));
    }
  }

  TransformationSchema::View transformations_view(&transformations);
  SpringsSchema::View springs_view(&springs);

  auto spring = System([=](Frame* frame) {
    auto* el = frame->result<SpringsSchema::View::Element>();

    Handle handle = el->key;
    Transformation accum = transformations_view[handle];

    for (const auto& connection : el->component) {
      const Handle& other = std::get<0>(connection);
      const float& spring_k = std::get<1>(connection);
      Vec3 p = transformations_view[other].p - transformations_view[handle].p;
      accum.v += p * spring_k;
    }
    
    frame->result(Transformations::Element{ IndexedBy::HANDLE , handle, accum});
  });

  auto move = System([=](Frame* frame) {
    auto* el = frame->result<Transformations::Element>();

    float l = el->component.v.length();
    el->component.v.normalize();
    el->component.v *= std::min(l, 10.0f);
    el->component.p += el->component.v;
  });

  auto bound_position = System([=](Frame* frame) {
    auto* el = frame->result<Transformations::Element>();

    if (el->component.p.x <= 0 || el->component.p.x >= MAX_WIDTH - 1) {
      el->component.p.x = std::min(
          std::max(el->component.p.x, 0.0f), (float)(MAX_WIDTH - 1));
      el->component.v.x *= -0.9f;
    }

    if (el->component.p.y <= 0 || el->component.p.y >= MAX_HEIGHT - 1) {
      el->component.p.y = std::min(
          std::max(el->component.p.y, 0.0f), (float)(MAX_HEIGHT - 1));
      el->component.v.y *= -0.9f;
    }
  });

  auto physics_pipeline = TransformationSchema::make_system_queue();

  Pipeline<SpringsSchema::View, Transformations> springs_pipeline(
    &springs_view, &transformations, spring, IndexedBy::KEY);
  Pipeline<Transformations, Transformations> transformations_pipeline(
    &transformations, &transformations, bound_position * move,
    IndexedBy::OFFSET);

  WindowTimer fps(60);
  uint64_t frame = 0;
  while (1) {
    fps.start();

    if (frame % 1 == 0) {
      physics_pipeline({ springs_pipeline, transformations_pipeline });
    } else {
      physics_pipeline({ transformations_pipeline });
    }
    
    fps.stop();
    fps.step();

    std::cout << frame << ": " << fps.get_avg_elapsed_ns()/1e6 << "\r";
    std::cout.flush();
    ++frame;
  }
  while (1);
}
#endif

struct Position {
  float x, y;
};

typedef radiance::Schema<uint32_t, Position> PositionSchema;
typedef PositionSchema::Table Positions;

int main() {
  Positions positions;
  uint32_t entity_count = 10000;
  for (uint32_t i = 0; i < entity_count; ++i) {
    positions.insert(i, { (float)(rand() % 1000), (float)(rand() % 1000)});
  }

  auto system_1 = radiance::System([](radiance::Frame* frame) {
    auto* el = frame->result<Positions::Element>();
    ++el->component.x;
    --el->component.y;
  });

  auto system_2 = radiance::System([](radiance::Frame* frame) {
    auto* el = frame->result<Positions::Element>();
    ++el->component.x;
    ++el->component.y;
  });

  auto system_3 = radiance::System([](radiance::Frame* frame) {
    auto* el = frame->result<Positions::Element>();
    --el->component.x;
    --el->component.y;
  });

  auto system = system_1 * system_2 * system_3;
  
  WindowTimer fps(60);
  uint64_t frame = 0;
  uint32_t max_iterations = 10000;
  while (frame < max_iterations) {
    fps.start();

    radiance::Pipeline<Positions, Positions>::run(
        &positions,
        &positions,
        system,
        radiance::IndexedBy::OFFSET
    );
    
    fps.stop();
    fps.step();

    std::cout << frame << ": "
      << (1000 * entity_count) / (fps.get_avg_elapsed_ns()/1e6) << "\r";
    std::cout.flush();
    ++frame;
  }
  std::cout << "\n";
  for (auto& c : positions.components) {
    std::cout << "(" << c.x << ", " << c.y << ")\r";
  }
  std::cout << "\n";
  std::cout << "Throughput = "
    << (int)((1000 * entity_count) / (fps.get_avg_elapsed_ns()/1e6))
    << "[entity/s]\n";
  return 0;
}
