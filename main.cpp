#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>


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

static int init_pend(const int N, double *theta0s, double *theta1s, double *theta2s){
  int n;
  for(n=0; n<N; n++){
    theta0s[n]=M_PI*0.5+0.01*(-0.5+rand()/RAND_MAX);
    theta1s[n]=0.;
    theta2s[n]=0.;
  }
  return 0;
}

static int update(const double dt, const int N, double *theta0s, double *theta1s, double *theta2s){
  const double g=1.;
  double *m=(double*)malloc(N*sizeof(double));
  double *l=(double*)malloc(N*sizeof(double));
  double *A=(double*)malloc(N*N*sizeof(double));
  double *B=(double*)malloc(N*N*sizeof(double));
  double *invA=(double*)malloc(N*N*sizeof(double));
  double *theta0s_old=(double*)malloc(N*sizeof(double));
  double *theta1s_old=(double*)malloc(N*sizeof(double));
  double residual=0.;
  const double residual_max=1.e-13;
  for(int i=0; i<N; i++){
    m[i]=1.;
    l[i]=1.;
    theta0s_old[i]=theta0s[i];
    theta1s_old[i]=theta1s[i];
  }
  do{
    for(int j=0; j<N; j++){
      for(int i=0; i<N; i++){
        A[j*N+i]=0.;
        B[j*N+i]=0.;
        invA[j*N+i]=0.;
      }
    }
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        for(int k=i<j?j:i; k<N; k++){
          A[i*N+j] += m[k];
        }
        A[i*N+j] *= l[j]*cos(0.5*(theta0s[i]+theta0s_old[i])-0.5*(theta0s[j]+theta0s_old[j]));
      }
    }
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        if(i==j){
          for(int k=i; k<N; k++){
            B[i*N+j] += m[k];
          }
          B[i*N+j] *= g*sin(0.5*(theta0s[i]+theta0s_old[i]));
        }else{
          for(int k=i<j?j:i; k<N; k++){
            B[i*N+j] += m[k];
          }
          B[i*N+j] *= l[j]*pow(0.5*(theta1s[j]+theta1s_old[j]), 2)*sin(0.5*(theta0s[i]+theta0s_old[i])-0.5*(theta0s[j]+theta0s_old[j]));
        }
      }
    }
    inverse(invA, A, N);
    for(int j=0; j<N; j++){
      for(int i=0; i<N; i++){
        A[j*N+i]=0.;
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
      theta1s[i]=theta1s_old[i]+theta2s[i]*dt;
      residual+=fabs(theta0s[i]-(theta0s_old[i]+0.5*(theta1s[i]+theta1s_old[i])*dt));
      theta0s[i]=theta0s_old[i]+0.5*(theta1s[i]+theta1s_old[i])*dt;
    }
    printf("%.1e\n", residual);
  }while(residual>residual_max);
  free(A);
  free(B);
  free(m);
  free(l);
  free(theta0s_old);
  free(theta1s_old);
  return 0;
}

int main(void){
  Display *display = NULL;
  Window window = NULL;
  XEvent event;
  GC gc;
  int screen;
  const int w_left   = 200;
  const int w_up     = 200;
  const int w_width  = 800;
  const int w_height = 800;
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
  unsigned int lbox = 50;
  XColor red, black, dummy;
  Colormap cmap;
  cmap = DefaultColormap(display, 0);
  XAllocNamedColor(display, cmap, "red", &red, &dummy);
  XAllocNamedColor(display, cmap, "black", &black, &dummy);
  const int N = 3;
  const double dt=0.01;
  double *theta0s=NULL;
  double *theta1s=NULL;
  double *theta2s=NULL;
  const short pend_ox = w_width/2;
  const short pend_oy = w_height/4;
  XArc *arcs=NULL;
  XSegment *lines=NULL;
  theta0s=(double*)malloc(sizeof(double)*N);
  theta1s=(double*)malloc(sizeof(double)*N);
  theta2s=(double*)malloc(sizeof(double)*N);
  lines=(XSegment*)malloc(sizeof(XSegment)*N);
  arcs=(XArc*)malloc(sizeof(XArc)*N);
  init_pend(N, theta0s, theta1s, theta2s);
  int iter=0;
  while(true){
    XClearWindow(display, window);
    update(dt, N, theta0s, theta1s, theta2s);
    if(iter%10==0){
      arcs[0].x = pend_ox+lbox*2*sin(theta0s[0])-lbox/2;
      arcs[0].y = pend_oy+lbox*2*cos(theta0s[0])-lbox/2;
      arcs[0].width = lbox;
      arcs[0].height = lbox;
      arcs[0].angle1 = 0;
      arcs[0].angle2 = 360*64;
      for(int n=1; n<N; n++){
        arcs[n].x = arcs[n-1].x+lbox*2*sin(theta0s[n]);
        arcs[n].y = arcs[n-1].y+lbox*2*cos(theta0s[n]);
        arcs[n].width = lbox;
        arcs[n].height = lbox;
        arcs[n].angle1 = 0;
        arcs[n].angle2 = 360*64;
      }
      lines[0].x1 = pend_ox;
      lines[0].y1 = pend_oy;
      lines[0].x2 = arcs[0].x+lbox/2;
      lines[0].y2 = arcs[0].y+lbox/2;
      for(int n=1; n<N; n++){
        lines[n].x1 = arcs[n-1].x+lbox/2;
        lines[n].y1 = arcs[n-1].y+lbox/2;
        lines[n].x2 = arcs[n  ].x+lbox/2;
        lines[n].y2 = arcs[n  ].y+lbox/2;
      }
      XSetForeground(display, gc, black.pixel);
      XDrawSegments(display, window, gc, lines, N);
      XFlush(display);
      XSetForeground(display, gc, red.pixel);
      XFillArcs(display, window, gc, arcs, N);
      XFlush(display);
      /* double ke=0.; */
      /* double pe=0.; */
      /* double te=0.; */
      /* for(int n=0; n<N; n++){ */
      /*   ke+= */
      /* } */
      /* char kest[64]={0}; */
      /* char pest[64]={0}; */
      /* char test[64]={0}; */
      /* sprintf(kest, "kinetic energy: %.7f\n", ke); */
      /* XDrawString(display, window, gc, 10, 10, kest, 64); */
      /* XDrawString(display, window, gc, 10, 30, pest, 64); */
      /* XDrawString(display, window, gc, 10, 50, test, 64); */
      /* XFlush(display); */
      usleep(3e3);
    }
  }
  free(theta0s);
  free(theta1s);
  free(theta2s);
  free(lines);
  free(arcs);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return 0;
}

