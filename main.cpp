#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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
    fprintf(stderr, "XOpenDisplay returns NULL\n");
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
  visual->pend_oy = window_height/4;
  return visual;
}

int update_visual(visual_t *visual, pendulum_t *pendulum){
  const int N = pendulum->N;
  const double *thetas=pendulum->theta0s;
  const double *ls=pendulum->ls;
  unsigned int lbox = 50;
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

int main(void){
  const int N = 3;
  const double dt = 0.01;
  const int stepmax = 10000;
  const int window_left   = 200;
  const int window_up     = 200;
  const int window_width  = 800;
  const int window_height = 800;
  visual_t *visual = init_visual(N, window_left, window_up, window_width, window_height);
  pendulum_t *pendulum = init_pendulum(N);
  for(int step=0; step<stepmax; step++){
    update_pendulum(dt, pendulum);
    if(step%10==0){
      update_visual(visual, pendulum);
      check_energy(pendulum);
      usleep(3.333e4);
    }
  }
  return 0;
}

