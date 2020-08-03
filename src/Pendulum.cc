#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include "Pendulum.h"

/*
 * Numerically integrate N-mass pendulum system in time
 * Reference:
 * https://note.com/sciencecafe_mc2/n/ne5077e5a62f1 (in Japanese)
 */

static void inverse_matrix(std::vector<double> &invA, std::vector<double> A, const int N);

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
}

Pendulum::~Pendulum(){
  /*
   * destructor, for now nothing to do
   */
}

void Pendulum::read_file(const std::string input_fname){
  /*
   * set initial conditions of pendulum
   */
  std::ifstream fp(input_fname);
  // temporary variables to store info from file
  double mass, length, theta, omega, alpha;
  std::string sbuf;
  // open file to read
  if(!fp.is_open()){
    std::cerr << "file open error: " << input_fname << std::endl;
    exit(EXIT_FAILURE);
  }
  // skip the 1st line since it's a header
  fp >> sbuf >> sbuf >> sbuf >> sbuf >> sbuf;
  this->N = 0;
  while(fp >> mass >> length >> theta >> omega >> alpha){
    masses.push_back(mass);
    lengths.push_back(length);
    thetas.push_back(theta);
    omegas.push_back(omega);
    alphas.push_back(alpha);
    this->N++; // increment the number of mass
  }
  std::cout << "Totally " << this->N << " masses are read" << std::endl;
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

void Pendulum::update(const double dt){
  /*
   * integrate the Lagrange equation in time (from t to t+dt)
   * and update mass position/velocity/accerelation
   * NOTE: for stability purpose, Crank-Nicolson implicit scheme is adopted
   */
  double residual;
  const double residual_max=1.e-15;
  // store previous values in *_old
  for(int n=0; n<N; n++){
    thetas_old[n] = thetas[n];
    omegas_old[n] = omegas[n];
  }
  // iterate until converge, Crank-Nicolson scheme
  do{
    // initialize working space
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        A[i*N+j]=0.;
        B[i*N+j]=0.;
        invA[i*N+j]=0.;
      }
    }
    // set A, see reference
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        for(int k=std::max(i, j); k<N; k++){
          A[i*N+j] += masses[k];
        }
        A[i*N+j] *= lengths[j]*cos(0.5*(thetas[i]+thetas_old[i])-0.5*(thetas[j]+thetas_old[j]));
      }
    }
    // set B, see reference
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        if(i==j){
          for(int k=i; k<N; k++){
            B[i*N+j] += masses[k];
          }
          B[i*N+j] *= g*sin(0.5*(thetas[i]+thetas_old[i]));
        }else{
          for(int k=std::max(i, j); k<N; k++){
            B[i*N+j] += masses[k];
          }
          B[i*N+j] *= lengths[j]*pow(0.5*(omegas[j]+omegas_old[j]), 2)*sin(0.5*(thetas[i]+thetas_old[i])-0.5*(thetas[j]+thetas_old[j]));
        }
      }
    }
    // the accleration "alpha" is obtained as
    // alpha = invA B e, where e = (-1 -1 ... -1)
    inverse_matrix(invA, A, N);
    // store invA B in A
    for(int i=0; i<N; i++){
      for(int j=0; j<N; j++){
        A[i*N+j] = 0.;
      }
    }
    for(int j=0; j<N; j++){
      for(int k=0; k<N; k++){
        for(int i=0; i<N; i++){
          A[j*N+i] += invA[j*N+k]*B[k*N+i];
        }
      }
    }
    for(int j=0; j<N; j++){
      alphas[j] = 0.;
      for(int i=0; i<N; i++){
        alphas[j] -= A[j*N+i];
      }
    }
    // convergence check in terms of velocity
    residual = 0.;
    for(int i=0; i<N; i++){
      // residual is defined as the difference between old and new velocity
      residual += fabs(omegas[i]-(omegas_old[i]+alphas[i]*dt));
      omegas[i] = omegas_old[i]+alphas[i]*dt;
      thetas[i] = thetas_old[i]+0.5*(omegas[i]+omegas_old[i])*dt;
    }
  }while(residual > residual_max*N);
}

void Pendulum::check_energy(const int step, const double time, const std::string energy_fname){
  /*
   * compute kinetic energy and potential energy and save it to file
   */
  std::ofstream fp(energy_fname, std::ios::app);
  double ke = 0.;
  double pe = 0.;
  double xdot, ydot;
  double y;
  if(!fp.is_open()){
    std::cerr << "file open error: " << energy_fname << std::endl;
    exit(EXIT_FAILURE);
  }
  for(int n=0; n<N; n++){
    y = 0.;
    xdot = 0.;
    ydot = 0.;
    for(int nn=0; nn<=n; nn++){
      y -= lengths[nn]*cos(thetas[nn]);
      xdot += lengths[nn]*omegas[nn]*cos(thetas[nn]);
      ydot += lengths[nn]*omegas[nn]*sin(thetas[nn]);
    }
    // U = m g y
    pe += masses[n]*g*y;
    // T = 1/2 m v^2
    ke += 0.5*masses[n]*(pow(xdot, 2.)+pow(ydot, 2.));
  }
  // write out to file
  fp << step << " " << std::fixed << std::setprecision(5) << time << " " << ke << " " << pe << std::endl;
}

static void inverse_matrix(std::vector<double> &invA, std::vector<double> A, const int N){
  /*
   * compute inverse matrix of A using Gaussian elimination
   * NOTE: O(N^3)
   */
  double tmp; // working variable
  // initialize invA with an identity matrix
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      if(i == j){
        invA[i*N+j] = 1.;
      }else{
        invA[i*N+j] = 0.;
      }
    }
  }
  // Gaussian elimination
  for(int i=0; i<N; i++){
    tmp = A[i*N+i];
    if(fabs(tmp) < std::numeric_limits<double>::epsilon()){
      std::cerr << "singular matrix: denominator is very small" << std::endl;
      exit(EXIT_FAILURE);
    }
    tmp = 1./tmp;
    for(int j=0; j<N; j++){
      A[i*N+j]    *= tmp;
      invA[i*N+j] *= tmp;
    }
    for(int j=0; j<N; j++){
      if(i != j){
        tmp = A[j*N+i];
        for(int k=0; k<N; k++){
          A[j*N+k]    -= A[i*N+k]*tmp;
          invA[j*N+k] -= invA[i*N+k]*tmp;
        }
      }
    }
  }
}

