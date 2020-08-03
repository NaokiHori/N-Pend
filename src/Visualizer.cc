#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <thread>
#include "Visualizer.h"

/*
 * Visualize pendulum using XWindow system
 * Reference:
 * https://www.x.org/wiki/guide/
 * http://rio.la.coocan.jp/lab/xlib/005graphics.htm
 */

Visualizer::Visualizer(const int N, const std::array<int, 4> window_position, const std::array<unsigned int, 2> pendulum_origin){
  /*
   * constructor
   */
  int left, top;
  int screen;
  Colormap cmap;
  // unpack input argument
  left         = window_position[0];
  top          = window_position[1];
  width        = window_position[2];
  height       = window_position[3];
  pend_originx = pendulum_origin[0];
  pend_originy = pendulum_origin[1];
  // create display connection
  display = XOpenDisplay(NULL);
  if(display == NULL){
    std::cerr <<  "XOpenDisplay returns NULL" << std::endl;
    exit(EXIT_FAILURE);
  }
  screen = DefaultScreen(display);
  // create window
  window = XCreateSimpleWindow(
      /* display   */ display,
      /* window    */ RootWindow(display, screen),
      /* x         */ left,
      /* y         */ top,
      /* width     */ width,
      /* height    */ height,
      /* border_w  */ 1,
      /* border    */ BlackPixel(display, screen),
      /* backgroud */ WhitePixel(display, screen)
  );
  XStoreName(display, window, "pendulum");
  XMapWindow(display, window);
  // graphic context
  gc = XCreateGC(display, window, 0, 0);
  cmap = DefaultColormap(display, 0);
  XAllocNamedColor(display, cmap, "red",   &(red),   &(dummy));
  XAllocNamedColor(display, cmap, "black", &(black), &(dummy));
  // allocate memory for drawing objects
  // only c pointers are accepted, so use classical malloc here
  lines = (XSegment*)malloc(sizeof(XSegment)*N);
  if(lines == NULL){
    std::cerr << "memory allocation failure: lines" << std::endl;
    exit(EXIT_FAILURE);
  }
  arcs = (XArc*)malloc(sizeof(XArc)*N);
  if(arcs == NULL){
    std::cerr << "memory allocation failure: arcs" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Visualizer::update(Pendulum pendulum, const double time){
  // load necessary info to draw from pendulum class
  const int N                       = pendulum.get_N();
  const std::vector<double> thetas  = pendulum.get_thetas();
  const std::vector<double> lengths = pendulum.get_lengths();
  // length of box containing a mass
  // configure nicely so that they fit in the window
  unsigned int lbox = (unsigned int)(height/N/2);
  // string stream object to write out time (and its position)
  std::stringstream ss_time;
  const int time_x = 10;
  const int time_y = 10;
  // set pendulum position to draw
  // NOTE: assume distance from one to another pendulum is lbox x 2
  for(int n=0; n<N; n++){
    if(n == 0){
      // need to give the left-top corner of a box containing a mass
      // so subtract lbox/2 at last
      arcs[n].x = pend_originx+lbox*2*lengths[n]*sin(thetas[n])-lbox/2;
      arcs[n].y = pend_originy+lbox*2*lengths[n]*cos(thetas[n])-lbox/2;
    }else{
      arcs[n].x = arcs[n-1].x+lbox*2*lengths[n]*sin(thetas[n]);
      arcs[n].y = arcs[n-1].y+lbox*2*lengths[n]*cos(thetas[n]);
    }
    arcs[n].width  = lbox;
    arcs[n].height = lbox;
    // circle, from 0 to 360 degree
    // the unit is 1/64 degree, so multiply 64
    arcs[n].angle1 =   0*64;
    arcs[n].angle2 = 360*64;
  }
  // set lines from one mass to next
  for(int n=0; n<N; n++){
    if(n == 0){
      // from origin to mass No. 0
      lines[n].x1 = pend_originx;
      lines[n].y1 = pend_originy;
    }else{
      // the mass position is given by left-top corner of a box containing a mass
      // so add lbox/2
      lines[n].x1 = arcs[n-1].x+lbox/2;
      lines[n].y1 = arcs[n-1].y+lbox/2;
    }
    lines[n].x2 = arcs[n].x+lbox/2;
    lines[n].y2 = arcs[n].y+lbox/2;
  }
  // clear window before drawing
  XClearWindow(display, window);
  // draw black lines & time
  XSetForeground(display, gc, black.pixel);
  XDrawSegments(display, window, gc, lines, N);
  ss_time << "time " << std::fixed << std::setprecision(5) << time;
  XDrawString(display, window, gc, time_x, time_y, ss_time.str().c_str(), ss_time.str().length());
  // draw red masses
  XSetForeground(display, gc, red.pixel);
  XFillArcs(display, window, gc, arcs, N);
  XFlush(display);
  // sleep for 33 mili-second to achieve 30 fps
  std::this_thread::sleep_for(std::chrono::milliseconds(33));
}

Visualizer::~Visualizer(){
  /*
   * destructor
   */
  free(lines);
  free(arcs);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}

