#include "inc/radiance.h"
#include "inc/table.h"
#include "inc/stack_memory.h"
#include "inc/schema.h"
#include "inc/timer.h"

#define GL3_PROTOTYPES 1

#include <GL/glew.h>
#include<stdio.h>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

#include "particles.h"

#include <unordered_map>

void add_render_pipeline() {
  radiance::Pipeline* render_pipeline =
      radiance::add_pipeline(kMainProgram, kParticleCollection, nullptr);
  render_pipeline->select = nullptr;
  render_pipeline->transform = [](radiance::Stack* s){
    Particles::Element* el =
        (Particles::Element*)((radiance::Mutation*)(s->top()))->element;
    glPushMatrix();
    glTranslatef(el->value.p.x, el->value.p.y, 0);
    glBegin(GL_POINTS);
    glColor3f(1., 0., 0.); glVertex3f(0, 0, 0);
    glEnd();
    glPopMatrix();
  };

  radiance::ExecutionPolicy policy;
  policy.priority = radiance::MIN_PRIORITY;
  policy.trigger = radiance::Trigger::LOOP;
  enable_pipeline(render_pipeline, policy);
}

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;
int main(int, char* []) {

  dpy = XOpenDisplay(NULL);

  if(dpy == NULL) {
    printf("\n\tcannot connect to X server\n\n");
    exit(0);
  }

  root = DefaultRootWindow(dpy);

  vi = glXChooseVisual(dpy, 0, att);

  if(vi == NULL) {
    printf("\n\tno appropriate visual found\n\n");
    exit(0);
  } 
  else {
    printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
  }


  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;

  win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

  XMapWindow(dpy, win);
  XStoreName(dpy, win, "VERY SIMPLE APPLICATION");

  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  glEnable(GL_DEPTH_TEST); 

  radiance::Universe uni;
  radiance::init(&uni);
  radiance::create_program(kMainProgram); 

  uint64_t particle_count = 1000;
  add_particle_collection(kParticleCollection, particle_count);
  add_particle_pipeline(kParticleCollection);
  add_render_pipeline();

  radiance::start();
  while (1) {
    if (XCheckWindowEvent(dpy, win, KeyPressMask, &xev)) {
      if (xev.type == KeyPress) {
        glXMakeCurrent(dpy, None, NULL);
        glXDestroyContext(dpy, glc);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        radiance::stop();
        exit(0);
      }
    }
    XGetWindowAttributes(dpy, win, &gwa);
    glViewport(0, 0, gwa.width, gwa.height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    radiance::loop();
    glXSwapBuffers(dpy, win);
  }
}
