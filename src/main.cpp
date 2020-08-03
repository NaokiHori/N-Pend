#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <string>
#include "Pendulum.h"
#include "Visualizer.h"


int main(){
  /***** configuration *****/
  // gravity
  const double g = 9.8;
  // time step size
  const double dt = 0.0001;
  // current time
  double time = 0.;
  // max iteration
  const int stepmax = 100000;
  // per each step update window infomation
  const int update_window_per_step = 100;
  // file name containing initial condition of pendulum
  const std::string input_fname("input.txt");
  // file name to output energy
  const std::string energy_fname("energy.txt");
  // window position, left, top, width, height, respectively
  const std::array<int, 4> window_position = {200, 200, 800, 400};
  const std::array<unsigned int, 2> pendulum_origin = {400, 20};
  /***** call constructors *****/
  Pendulum pendulum(input_fname, g);
  Visualizer visualizer(pendulum.get_N(), window_position, pendulum_origin);
  std::cout << "start simulating" << std::endl;
  /***** main loop *****/
  for(int step=0; step<stepmax; step++){
    // integrate in time
    pendulum.update(dt);
    if(step%update_window_per_step == 0){
      // update window (drawing new mass positions)
      visualizer.update(pendulum, time);
      // total energy should be conserved, so check it
      pendulum.check_energy(step, time, energy_fname);
    }
    time += dt;
  }
  std::cout << "finish simulating" << std::endl;
  return 0;
}

