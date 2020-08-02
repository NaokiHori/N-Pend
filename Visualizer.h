#if !defined(VISUALIZER_H)
#define VISUALIZER_H

#include <X11/Xlib.h>
#include "Pendulum.h"

class Visualizer {
  public:
    Visualizer(const int, const std::array<int, 4>);
    ~Visualizer();
    int update(class Pendulum, const double);
  private:
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
};

#endif // VISUALIZER_H
