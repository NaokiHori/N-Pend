#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <string>
#include <chrono>
#include <thread>
#include "Pendulum.h"
#include "Visualizer.h"


int main(){
  // gravity
  const double g = 9.8;
  // time step size
  const double dt = 0.0001;
  // current time
  double time=0.;
  // max iteration
  const int stepmax = 100000;
  // per each step update window infomation
  const int update_window_per_step = 100;
  // file name containing initial condition of pendulum
  const std::string input_fname("input.txt");
  // window position, left, top, width, height, respectively
  const std::array<int, 4> window_position = {200, 200, 800, 800};
  /***** call constructors *****/
  Pendulum pendulum(input_fname, g);
  Visualizer visualizer(pendulum.get_N(), window_position);
  std::cout << "start simulating" << std::endl;
  /***** main loop *****/
  for(int step=0; step<stepmax; step++){
    pendulum.update(dt);
    if(step%update_window_per_step==0){
      visualizer.update(pendulum, time);
      // sleep for 33 mili-second to achieve 30 fps
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    /* pendulum.check_energy(); */
    time += dt;
  }
  std::cout << "finish simulating" << std::endl;
  return 0;
}

