#if !defined(VISUALIZER_H)
#define VISUALIZER_H

#include <X11/Xlib.h>
#include "Pendulum.h"

class Visualizer {
  public:
    // constructor
    Visualizer(const std::vector<class Pendulum> pendulums, const std::array<int, 4> window_position, const std::array<unsigned int, 2> pendulum_origin, std::vector<const std::string> colors);
    // destructor
    ~Visualizer();
    void update(std::vector<class Pendulum> pendulums, const double dt);
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
    XColor black, dummy;
    XColor *mcolors;
};

#endif // VISUALIZER_H
