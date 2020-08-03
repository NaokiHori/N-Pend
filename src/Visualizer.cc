#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <cmath>
#include "Visualizer.h"


Visualizer::Visualizer(const int N, const std::array<int, 4> window_position){
  display = XOpenDisplay(NULL);
  if(display == NULL){
    std::cerr <<  "XOpenDisplay returns NULL" << std::endl;
    exit(EXIT_FAILURE);
  }
  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(
      display,
      RootWindow(display, screen),
      window_position[0],
      window_position[1],
      window_position[2],
      window_position[3],
      1,
      BlackPixel(display, screen),
      WhitePixel(display, screen)
  );
  XStoreName(display, window, "window name");
  XMapWindow(display, window);
  gc = XCreateGC(display, window, 0, 0);
  cmap = DefaultColormap(display, 0);
  XAllocNamedColor(display, cmap, "red",   &(red),   &(dummy));
  XAllocNamedColor(display, cmap, "black", &(black), &(dummy));
  lines=(XSegment*)malloc(sizeof(XSegment)*N);
  arcs=(XArc*)malloc(sizeof(XArc)*N);
  pend_ox = window_position[2]/2;
  pend_oy = window_position[3]/8;
}

int Visualizer::update(Pendulum pendulum, const double time){
  const int N = pendulum.get_N();
  const std::vector<double> thetas=pendulum.get_thetas();
  const std::vector<double> lengths=pendulum.get_lengths();
  unsigned int lbox = 75;
  for(int n=0; n<N; n++){
    if(n==0){
      arcs[n].x = pend_ox+lbox*2*lengths[n]*sin(thetas[n])-lbox/2;
      arcs[n].y = pend_oy+lbox*2*lengths[n]*cos(thetas[n])-lbox/2;
    }else{
      arcs[n].x = arcs[n-1].x+lbox*2*lengths[n]*sin(thetas[n]);
      arcs[n].y = arcs[n-1].y+lbox*2*lengths[n]*cos(thetas[n]);
    }
    arcs[n].width = lbox;
    arcs[n].height = lbox;
    arcs[n].angle1 = 0;
    arcs[n].angle2 = 360*64;
  }
  for(int n=0; n<N; n++){
    if(n==0){
      lines[n].x1 = pend_ox;
      lines[n].y1 = pend_oy;
    }else{
      lines[n].x1 = arcs[n-1].x+lbox/2;
      lines[n].y1 = arcs[n-1].y+lbox/2;
    }
    lines[n].x2 = arcs[n].x+lbox/2;
    lines[n].y2 = arcs[n].y+lbox/2;
  }
  XClearWindow(display, window);
  XSetForeground(display, gc, black.pixel);
  XDrawSegments(display, window, gc, lines, N);
  char time_string[18];
  sprintf(time_string, "time %7.3f\n", time);
  XDrawString(display, window, gc, 10, 10, time_string, strlen(time_string));
  XFlush(display);
  XSetForeground(display, gc, red.pixel);
  XFillArcs(display, window, gc, arcs, N);
  XFlush(display);
  return 0;
}

Visualizer::~Visualizer(){
  free(lines);
  free(arcs);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

