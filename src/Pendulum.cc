#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include "Pendulum.h"


static int inverse(std::vector<double> &invA, std::vector<double> A, const int N);

Pendulum::Pendulum(const std::string input_fname, const double g){
  /*
   * constructor
   */
  read_file(input_fname); // N is set inside this function
  this->g = g;
  // allocate memory of working vars
  thetas_old = std::vector<double>(N,   0.);
  omegas_old = std::vector<double>(N,   0.);
  A          = std::vector<double>(N*N, 0.);
  B          = std::vector<double>(N*N, 0.);
  invA       = std::vector<double>(N*N, 0.);
  std::cout << "pendulum initialization finished" << std::endl;
}

Pendulum::~Pendulum(){
  /*
   * destructor, for now nothing to do
   */
}

int Pendulum::read_file(const std::string input_fname){
  /*
   * set initial conditions of pendulum
   */
  std::ifstream fp(input_fname);
  // temporary variables to store info from file
  double mass, length, theta, omega, alpha;
  std::string sbuf;
  // open file
  if(!fp.is_open()){
    std::cerr << "file open error: " << input_fname << std::endl;
    exit(EXIT_FAILURE);
  }
  // skip header
  fp >> sbuf >> sbuf >> sbuf >> sbuf >> sbuf;
  this->N = 0;
  while(fp >> mass >> length >> theta >> omega >> alpha){
    masses.push_back(mass);
    lengths.push_back(length);
    thetas.push_back(theta);
    omegas.push_back(omega);
    alphas.push_back(alpha);
    this->N++;
  }
  std::cout << "Load initial condition finished" << std::endl;
  std::cout << "Totally " << this->N << " masses are read" << std::endl;
  return 0;
}

const int Pendulum::get_N(){
  return N;
}

const std::vector<double> Pendulum::get_thetas(){
  return thetas;
}

const std::vector<double> Pendulum::get_lengths(){
  return lengths;
}

int Pendulum::update(const double dt){
  double residual;
  const double residual_max=1.e-15;
  for(int i=0; i<N; i++){
    thetas_old[i]=thetas[i];
    omegas_old[i]=omegas[i];
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
          A[i*N+j] += masses[k];
        }
        A[i*N+j] *= lengths[j]*cos(0.5*(thetas[i]+thetas_old[i])-0.5*(thetas[j]+thetas_old[j]));
      }
    }
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        if(i==j){
          for(int k=i; k<N; k++){
            B[i*N+j] += masses[k];
          }
          B[i*N+j] *= g*sin(0.5*(thetas[i]+thetas_old[i]));
        }else{
          for(int k=i<j?j:i; k<N; k++){
            B[i*N+j] += masses[k];
          }
          B[i*N+j] *= lengths[j]*pow(0.5*(omegas[j]+omegas_old[j]), 2)*sin(0.5*(thetas[i]+thetas_old[i])-0.5*(thetas[j]+thetas_old[j]));
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
      alphas[j]=0.;
      for(int i=0; i<N; i++){
        alphas[j]+=A[j*N+i];
      }
    }
    residual=0.;
    for(int i=0; i<N; i++){
      residual+=fabs(omegas[i]-(omegas_old[i]+alphas[i]*dt));
      omegas[i]=omegas_old[i]+alphas[i]*dt;
      thetas[i]=thetas_old[i]+0.5*(omegas[i]+omegas_old[i])*dt;
    }
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
      y -= lengths[nn]*cos(thetas[nn]);
      xdot += lengths[nn]*omegas[nn]*cos(thetas[nn]);
      ydot += lengths[nn]*omegas[nn]*sin(thetas[nn]);
    }
    pe += masses[n]*g*y;
    ke += 0.5*masses[n]*(pow(xdot, 2.)+pow(ydot, 2.));
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

