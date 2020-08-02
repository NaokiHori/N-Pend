#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <X11/Xlib.h>
#include "Pendulum.h"


typedef struct {
  Display *display;
  Window window;
  GC gc;
  int screen;
  XArc *arcs;
  XSegment *lines;
  XColor red, black, dummy;
  Colormap cmap;
  short pend_ox;
  short pend_oy;
} visual_t;

visual_t *init_visual(const int N, const int window_left, const int window_up, const int window_width, const int window_height){
  visual_t *visual=(visual_t*)malloc(sizeof(visual_t));
  visual->display = XOpenDisplay(NULL);
  if(visual->display == NULL){
    std::cerr <<  "XOpenDisplay returns NULL" << std::endl;
    exit(EXIT_FAILURE);
  }
  visual->screen = DefaultScreen(visual->display);
  visual->window = XCreateSimpleWindow(
      visual->display,
      RootWindow(visual->display, visual->screen),
      window_left,
      window_up,
      window_width,
      window_height,
      1,
      BlackPixel(visual->display, visual->screen),
      WhitePixel(visual->display, visual->screen)
  );
  XStoreName(visual->display, visual->window, "window name");
  XMapWindow(visual->display, visual->window);
  visual->gc = XCreateGC(visual->display, visual->window, 0, 0);
  visual->cmap = DefaultColormap(visual->display, 0);
  XAllocNamedColor(visual->display, visual->cmap, "red",   &(visual->red),   &(visual->dummy));
  XAllocNamedColor(visual->display, visual->cmap, "black", &(visual->black), &(visual->dummy));
  visual->lines=(XSegment*)malloc(sizeof(XSegment)*N);
  visual->arcs=(XArc*)malloc(sizeof(XArc)*N);
  visual->pend_ox = window_width/2;
  visual->pend_oy = window_height/2;
  return visual;
}

int update_visual(visual_t *visual, Pendulum pendulum){
  const int N = pendulum.get_N();
  const std::vector<double> thetas=pendulum.get_theta0s();
  const std::vector<double> ls=pendulum.get_ls();
  unsigned int lbox = 75;
  XArc *arcs=visual->arcs;
  XSegment *lines=visual->lines;
  for(int n=0; n<N; n++){
    if(n==0){
      arcs[n].x = visual->pend_ox+lbox*2*ls[n]*sin(thetas[n])-lbox/2;
      arcs[n].y = visual->pend_oy+lbox*2*ls[n]*cos(thetas[n])-lbox/2;
    }else{
      arcs[n].x = arcs[n-1].x+lbox*2*ls[n]*sin(thetas[n]);
      arcs[n].y = arcs[n-1].y+lbox*2*ls[n]*cos(thetas[n]);
    }
    arcs[n].width = lbox;
    arcs[n].height = lbox;
    arcs[n].angle1 = 0;
    arcs[n].angle2 = 360*64;
  }
  for(int n=0; n<N; n++){
    if(n==0){
      lines[n].x1 = visual->pend_ox;
      lines[n].y1 = visual->pend_oy;
    }else{
      lines[n].x1 = arcs[n-1].x+lbox/2;
      lines[n].y1 = arcs[n-1].y+lbox/2;
    }
    lines[n].x2 = arcs[n].x+lbox/2;
    lines[n].y2 = arcs[n].y+lbox/2;
  }
  XClearWindow(visual->display, visual->window);
  XSetForeground(visual->display, visual->gc, visual->black.pixel);
  XDrawSegments(visual->display, visual->window, visual->gc, visual->lines, N);
  XFlush(visual->display);
  XSetForeground(visual->display, visual->gc, visual->red.pixel);
  XFillArcs(visual->display, visual->window, visual->gc, visual->arcs, N);
  XFlush(visual->display);
  return 0;
}

int destruct_visual(visual_t *visual){
  free(visual->lines);
  free(visual->arcs);
  XDestroyWindow(visual->display, visual->window);
  XCloseDisplay(visual->display);
  free(visual);
  return 0;
}

int main(){
  const int N = 2;
  const double g = 1.;
  const double dt = 0.01;
  const int stepmax = 10000;
  const int window_left   = 200;
  const int window_up     = 200;
  const int window_width  = 800;
  const int window_height = 800;
  visual_t *visual = init_visual(N, window_left, window_up, window_width, window_height);
  Pendulum pendulum(N, g);
  for(int step=0; step<stepmax; step++){
    pendulum.update(dt);
    if(step%10==0){
      update_visual(visual, pendulum);
      pendulum.check_energy();
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
  }
  return 0;
}

