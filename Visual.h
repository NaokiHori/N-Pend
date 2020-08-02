#if !defined(VISUAL_H)
#define VISUAL_H

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

visual_t *init_visual(const int N, const std::array<int, 4> window_position);
int update_visual(visual_t *visual, Pendulum pendulum);
int destruct_visual(visual_t *visual);

#endif // VISUAL_H
