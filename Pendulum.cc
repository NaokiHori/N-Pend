#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <cmath>
#include "Pendulum.h"


static int inverse(std::vector<double> &invA, std::vector<double> A, const int N);

Pendulum::Pendulum(const int N, const double g){
  this->N = N;
  this->g = g;
  theta0s     = std::vector<double>(N,   0.);
  theta1s     = std::vector<double>(N,   0.);
  theta2s     = std::vector<double>(N,   0.);
  theta0s_old = std::vector<double>(N,   0.);
  theta1s_old = std::vector<double>(N,   0.);
  ms          = std::vector<double>(N,   0.);
  ls          = std::vector<double>(N,   0.);
  A           = std::vector<double>(N*N, 0.);
  B           = std::vector<double>(N*N, 0.);
  invA        = std::vector<double>(N*N, 0.);
  for(int n=0; n<N; n++){
    theta0s[n] = M_PI*1.0+1.e-4*(-0.5+rand()/RAND_MAX);
    ms[n] = 1.;
    ls[n] = 1.;
  }
}

Pendulum::~Pendulum(){
}

const int Pendulum::get_N(){
  return N;
}

const std::vector<double> Pendulum::get_theta0s(){
  return theta0s;
}

const std::vector<double> Pendulum::get_ls(){
  return ls;
}

int Pendulum::update(const double dt){
  double residual;
  const double residual_max=1.e-15;
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
    std::clog << "residual " << std::scientific << residual << std::endl;
  }while(residual>residual_max*N);
  return 0;
}

int Pendulum::check_energy(){
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
  std::cout << "ke: " << ke <<  " pe: " << pe << " te: " << ke+pe << std::endl;
  return 0;
}

static int inverse(std::vector<double> &invA, std::vector<double> A, const int N){
  double buf;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      if(i == j){
        invA[i*N+j] = 1.;
      }else{
        invA[i*N+j] = 0.;
      }
    }
  }
  for(int i=0; i<N; i++){
    buf = 1./A[i*N+i];
    for(int j=0; j<N; j++){
      A[i*N+j]    *= buf;
      invA[i*N+j] *= buf;
    }
    for(int j=0;j<N;j++){
      if(i!=j){
        buf=A[j*N+i];
        for(int k=0;k<N;k++){
          A[j*N+k]-=A[i*N+k]*buf;
          invA[j*N+k]-=invA[i*N+k]*buf;
        }
      }
    }
  }
  return 0;
}

