#if !defined(VISUALIZER_H)
#define VISUALIZER_H

#include <X11/Xlib.h>
#include "Pendulum.h"

class Visualizer {
  public:
    // constructor
    Visualizer(const int, const std::array<int, 4> window_position, const std::array<unsigned int, 2> pendulum_origin);
    // destructor
    ~Visualizer();
    void update(class Pendulum pendulum, const double dt);
  private:
    // window size
    int width;
    int height;
    // pendulum origin
    short pend_originx;
    short pend_originy;
    // Xwindow objects
    Display *display;
    Window window;
    GC gc;
    // objects to be drawn
    XArc *arcs;
    XSegment *lines;
    XColor red, black, dummy;
};

#endif // VISUALIZER_H
