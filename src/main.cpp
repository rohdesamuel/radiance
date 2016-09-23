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

#include <SDL2/SDL.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>

#include "radiance.h"

#include "timer.h"
#include "vector_math.h"

#include "component.h"
#include "benchmark.h"

int main() {
  SDL_Window* window = nullptr;
  SDL_GLContext context = nullptr;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cout << "Failed to initialize SDL2\n"
              << SDL_GetError();
    return 2;
  }

  window = SDL_CreateWindow(
      "Demo",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640,
      480,
      SDL_WINDOW_OPENGL);

  if (!window) {
    std::cout << "Failed to create window\n"
              << SDL_GetError();
    return 3;
  }

  context = SDL_GL_CreateContext(window);
  
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != 0) {
    std::cout << "Failed to intialize Glew\n"
              << "Error code: " << glewError;
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetSwapInterval(1);
  
  GLuint triangle;
  glGenVertexArrays(1, &triangle);
  glBindVertexArray(triangle);

  static const GLfloat triangle_data[] = {
     0.0f,  0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
  };

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), triangle_data,
               GL_STATIC_DRAW);

  const char* vertex_shader =
      "#version 130\n"
      "in vec3 vp;"
      "void main() {"
      "  gl_Position = vec4(vp, 1.0);"
      "}";

  const char* fragment_shader =
      "#version 130\n"
      "out vec4 frag_color;"
      "void main() {"
      "  frag_color = vec4(1.0, 0.0, 0.0, 1.0);"
      "}";

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  {
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);
    GLint success = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      GLint log_size = 0;
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &log_size);
      char* error = new char[log_size];

      glGetShaderInfoLog(vs, log_size, &log_size, error);
      std::cout << "Error in vertex shader compilation: " << error << std::endl;
      delete[] error;
    }
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  {
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);
    GLint success = 0;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      GLint log_size = 0;
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &log_size);
      char* error = new char[log_size];

      glGetShaderInfoLog(vs, log_size, &log_size, error);
      std::cout << "Error in fragment shader compilation: " << error << std::endl;
      delete[] error;
    }
  }

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glClearColor(0.0, 1.0, 0.0, 1.0);
  bool running = true;
  while(running) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }

      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            running = false;
            break;
        }
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(shader_program);

      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glVertexAttribPointer(
          0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glDrawArrays(GL_TRIANGLES, 0, 3);
      glDisableVertexAttribArray(0);

      SDL_GL_SwapWindow(window);

      GLenum error = glGetError();
      if (error) {
        std::cout << "Error: " << error << std::endl;
      }
    }
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

#if 0
using namespace radiance;

struct Transformation {
  Vec3 p;
  Vec3 v;
};
typedef ::boost::container::vector<std::tuple<Handle, float>> Spring;

typedef Schema<Handle, Spring> Springs;
typedef Schema<Entity, Transformation> Transformations;

template<class Data_>
struct Attribute {
  typedef Data_ Type;
  Handle handle;
};

typedef Attribute<Transformations::Element> Movable;

struct Particle {
  struct has {
    Handle transformation;
  };

  struct is {
    Handle renderable;
    Handle movable;
  };
};

typedef radiance::Schema<Id, System> Renderables;
typedef radiance::Schema<Id, System> Movables;

int main() {
  const int MAX_HEIGHT = 640 / 8;
  const int MAX_WIDTH = 640 / 8;

  Springs::Table springs;
  Transformations::Table transformations;
  
  Id movable_id = 0;
  Movables::Table movables;
  movables.insert(movable_id, [](Frame* frame) {
    auto *el = frame->result<Movable::Type>();
    el->component.p += el->component.v;
  });

  Transformations::MutationQueue transformations_queue;

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

  Transformations::View transformations_view(&transformations);
  Springs::View springs_view(&springs);

  auto spring = System([=](Frame* frame) {
    auto* el = frame->result<Springs::View::Element>();

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

  auto bind_position = System::compile(
    [](Frame* frame, int max_width, int max_height) {
      auto* el = frame->result<Transformations::Element>();

      if (el->component.p.x <= 0 || el->component.p.x >= max_width - 1) {
        el->component.p.x = std::min(
            std::max(el->component.p.x, 0.0f), (float)(max_width - 1));
        el->component.v.x *= -0.9f;
      }

      if (el->component.p.y <= 0 || el->component.p.y >= max_height - 1) {
        el->component.p.y = std::min(
            std::max(el->component.p.y, 0.0f), (float)(max_height - 1));
        el->component.v.y *= -0.9f;
      }
    }, MAX_WIDTH, MAX_HEIGHT);

  auto physics_pipeline = Transformations::make_system_queue();

  Pipeline<Springs::View, Transformations::Table> springs_pipeline(
      &springs_view,
      &transformations,
      {IndexedBy::KEY},
      {}, spring);

  Pipeline<Transformations::Table, Transformations::Table>
      transformations_pipeline(
          &transformations,
          &transformations,
          {IndexedBy::OFFSET},
          {}, bind_position * move);

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
#if 0
int main() {
  Benchmark benchmark(10000);
  benchmark.run();
}
#endif
