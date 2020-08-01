#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>


int main(void){
  Display *display = NULL;
  Window window = NULL;
  XEvent event;
  GC gc;
  int screen;
  const int w_left   = 200;
  const int w_up     = 200;
  const int w_width  = 900;
  const int w_height = 300;
  display = XOpenDisplay(NULL);
  if(display == NULL){
    fprintf(stderr, "XOpenDisplay returns NULL\n");
    exit(EXIT_FAILURE);
  }
  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(
      display, RootWindow(display, screen),
      w_left, w_up, w_width, w_height, 1,
      BlackPixel(display, screen),
      WhitePixel(display, screen)
  );
  XStoreName(display, window, "window name");
  XMapWindow(display, window);
  gc = XCreateGC( display, window, NULL, NULL );
  int x = 10;
  int y = 10;
  unsigned int width = 100;
  unsigned int height = 100;
  XColor color, dummy;
  Colormap cmap;
  cmap = DefaultColormap(display, 0);
  XAllocNamedColor(display, cmap, "red", &color, &dummy);
  XSetForeground(display, gc, color.pixel);
  while(true){
    XClearWindow(display, window);
    XFillArc(display, window, gc, x, y, width, height, 0, 360*64);
    XFlush(display);
    usleep(1e5);
    x += 10;
  }
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return 0;
}

