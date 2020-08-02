#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>


typedef struct {
  int N;
  double g;
  double *theta0s;
  double *theta1s;
  double *theta2s;
  double *ms;
  double *ls;
  /* temporary */
  double *A;
  double *B;
  double *invA;
  double *theta0s_old;
  double *theta1s_old;
} pendulum_t;

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

int inverse(double *inv_a, double *a, const int n){
  double buf;
  int i, j, k;
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      inv_a[i*n+j]=(i==j)?1.0:0.0;
    }
  }
  for(i=0;i<n;i++){
    buf=1/a[i*n+i];
    for(j=0;j<n;j++){
      a[i*n+j]*=buf;
      inv_a[i*n+j]*=buf;
    }
    for(j=0;j<n;j++){
      if(i!=j){
        buf=a[j*n+i];
        for(k=0;k<n;k++){
          a[j*n+k]-=a[i*n+k]*buf;
          inv_a[j*n+k]-=inv_a[i*n+k]*buf;
        }
      }
    }
  }
  return 0;
}

static pendulum_t *init_pendulum(const int N){
  pendulum_t *pendulum=(pendulum_t*)malloc(sizeof(pendulum_t));
  pendulum->N = N;
  pendulum->g = 1.;
  pendulum->theta0s = (double*)malloc(sizeof(double)*N);
  pendulum->theta1s = (double*)malloc(sizeof(double)*N);
  pendulum->theta2s = (double*)malloc(sizeof(double)*N);
  pendulum->ms      = (double*)malloc(sizeof(double)*N);
  pendulum->ls      = (double*)malloc(sizeof(double)*N);
  pendulum->A       = (double*)malloc(N*N*sizeof(double));
  pendulum->B       = (double*)malloc(N*N*sizeof(double));
  pendulum->invA        = (double*)malloc(N*N*sizeof(double));
  pendulum->theta0s_old = (double*)malloc(N*sizeof(double));
  pendulum->theta1s_old = (double*)malloc(N*sizeof(double));
  for(int n=0; n<N; n++){
    pendulum->theta0s[n]=M_PI*0.5+0.01*(-0.5+rand()/RAND_MAX);
    pendulum->theta1s[n]=0.;
    pendulum->theta2s[n]=0.;
    pendulum->ms[n]=.75*(n+1);
    pendulum->ls[n]=.75*(n+1);
  }
  return pendulum;
}

static int update_pendulum(const double dt, pendulum_t *pendulum){
  const int N = pendulum->N;
  const double g = pendulum->g;
  double *theta0s=pendulum->theta0s;
  double *theta1s=pendulum->theta1s;
  double *theta2s=pendulum->theta2s;
  const double *ms=pendulum->ms;
  const double *ls=pendulum->ls;
  double *A=pendulum->A;
  double *B=pendulum->B;
  double *invA=pendulum->invA;
  double *theta0s_old=pendulum->theta0s_old;
  double *theta1s_old=pendulum->theta1s_old;
  double residual=0.;
  const double residual_max=1.e-14;
  for(int i=0; i<N; i++){
    theta0s_old[i]=theta0s[i];
    theta1s_old[i]=theta1s[i];
  }
  do{
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        A[i*N+j]=0.;
        B[i*N+j]=0.;
        invA[i*N+j]=0.;
      }
    }
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        for(int k=i<j?j:i; k<N; k++){
          A[i*N+j] += ms[k];
        }
        A[i*N+j] *= ls[j]*cos(0.5*(theta0s[i]+theta0s_old[i])-0.5*(theta0s[j]+theta0s_old[j]));
      }
    }
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        if(i==j){
          for(int k=i; k<N; k++){
            B[i*N+j] += ms[k];
          }
          B[i*N+j] *= g*sin(0.5*(theta0s[i]+theta0s_old[i]));
        }else{
          for(int k=i<j?j:i; k<N; k++){
            B[i*N+j] += ms[k];
          }
          B[i*N+j] *= ls[j]*pow(0.5*(theta1s[j]+theta1s_old[j]), 2)*sin(0.5*(theta0s[i]+theta0s_old[i])-0.5*(theta0s[j]+theta0s_old[j]));
        }
      }
    }
    inverse(invA, A, N);
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        A[i*N+j]=0.;
      }
    }
    for(int j=0; j<N; j++){
      for(int i=0; i<N; i++){
        for(int k=0; k<N; k++){
          A[j*N+i]-=invA[j*N+k]*B[k*N+i];
        }
      }
    }
    for(int j=0; j<N; j++){
      theta2s[j]=0.;
      for(int i=0; i<N; i++){
        theta2s[j]+=A[j*N+i];
      }
    }
    residual=0.;
    for(int i=0; i<N; i++){
      residual+=fabs(theta1s[i]-(theta1s_old[i]+theta2s[i]*dt));
      theta1s[i]=theta1s_old[i]+theta2s[i]*dt;
      theta0s[i]=theta0s_old[i]+0.5*(theta1s[i]+theta1s_old[i])*dt;
    }
  }while(residual>residual_max);
  return 0;
}

static int check_energy(pendulum_t *pendulum){
  const int N = pendulum->N;
  const double g = pendulum->g;
  const double *theta0s=pendulum->theta0s;
  const double *theta1s=pendulum->theta1s;
  const double *ls=pendulum->ls;
  const double *ms=pendulum->ms;
  double ke = 0.;
  double pe = 0.;
  double xdot, ydot;
  double y;
  for(int n=0; n<N; n++){
    y = 0.;
    xdot = 0.;
    ydot = 0.;
    for(int nn=0; nn<=n; nn++){
      y -= ls[nn]*cos(theta0s[nn]);
      xdot += ls[nn]*theta1s[nn]*cos(theta0s[nn]);
      ydot += ls[nn]*theta1s[nn]*sin(theta0s[nn]);
    }
    pe += ms[n]*g*y;
    ke += 0.5*ms[n]*(pow(xdot, 2.)+pow(ydot, 2.));
  }
  printf("ke: %.7f pe: %.7f te: %.7f\n", ke, pe, ke+pe);
  return 0;
}

static int destruct_pendulum(pendulum_t *pendulum){
  free(pendulum->theta0s);
  free(pendulum->theta1s);
  free(pendulum->theta2s);
  free(pendulum->ms);
  free(pendulum->ls);
  free(pendulum->A);
  free(pendulum->B);
  free(pendulum->theta0s_old);
  free(pendulum->theta1s_old);
  free(pendulum);
  return 0;
}

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

