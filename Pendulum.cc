#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "Pendulum.h"


static int inverse(double *inv_a, double *a, const int n){
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

pendulum_t *init_pendulum(const int N){
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

int update_pendulum(const double dt, pendulum_t *pendulum){
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

int check_energy(pendulum_t *pendulum){
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
  std::cout << "ke: " << ke <<  "pe: " << pe << "te: " << ke+pe << std::endl;
  return 0;
}

int destruct_pendulum(pendulum_t *pendulum){
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

