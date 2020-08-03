#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <chrono>
#include <thread>
#include "Pendulum.h"
#include "Visualizer.h"


int main(){
  const int N = 2;
  const double g = 9.8;
  const double dt = 0.001;
  double time=0.;
  const int stepmax = 10000;
  /* left, top, width, height */
  const std::array<int, 4> window_position = {200, 200, 800, 800};
  Pendulum pendulum(N, g);
  Visualizer visualizer(N, window_position);
  std::cout << "start simulating" << std::endl;
  for(int step=0; step<stepmax; step++){
    pendulum.update(dt);
    if(step%10==0){
      visualizer.update(pendulum, time);
      pendulum.check_energy();
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    time += dt;
  }
  std::cout << "finish simulating" << std::endl;
  return 0;
}

