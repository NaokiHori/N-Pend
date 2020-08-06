#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include "Pendulum.h"
#include "Visualizer.h"


int main(){
  /***** configuration *****/
  // gravity
  const double g = 9.8;
  // time step size
  const double dt = 0.0002;
  // current time
  double time = 0.;
  // max iteration
  const int stepmax = 1000000;
  // per each step update window infomation
  const int update_window_per_step = 100;
  // file name containing initial condition of pendulum
  const std::string input_fname0("input0.txt");
  const std::string input_fname1("input1.txt");
  const std::string input_fname2("input2.txt");
  const std::string input_fname3("input3.txt");
  const std::string input_fname4("input4.txt");
  const std::string input_fname5("input5.txt");
  const std::string input_fname6("input6.txt");
  const std::string energy_fname("energy.txt");
  const std::string color0("rgb:FF/00/00");
  const std::string color1("rgb:FF/A5/00");
  const std::string color2("rgb:FF/FF/00");
  const std::string color3("rgb:00/80/00");
  const std::string color4("rgb:00/FF/FF");
  const std::string color5("rgb:00/00/FF");
  const std::string color6("rgb:80/00/80");
  // window position, left, top, width, height, respectively
  const std::array<int, 4> window_position = {0, 0, 640, 400};
  const std::array<unsigned int, 2> pendulum_origin = {320, 50};
  /***** call constructors *****/
  std::vector<class Pendulum> pendulums;
  std::vector<const std::string> colors;
  pendulums.push_back(Pendulum(input_fname0, g));
  pendulums.push_back(Pendulum(input_fname1, g));
  pendulums.push_back(Pendulum(input_fname2, g));
  pendulums.push_back(Pendulum(input_fname3, g));
  pendulums.push_back(Pendulum(input_fname4, g));
  pendulums.push_back(Pendulum(input_fname5, g));
  pendulums.push_back(Pendulum(input_fname6, g));
  colors.push_back(color0);
  colors.push_back(color1);
  colors.push_back(color2);
  colors.push_back(color3);
  colors.push_back(color4);
  colors.push_back(color5);
  colors.push_back(color6);
  Visualizer visualizer(pendulums, window_position, pendulum_origin, colors);
  std::cout << "start simulating" << std::endl;
  /***** main loop *****/
  for(int step=0; step<stepmax; step++){
    // integrate in time
    for(class Pendulum &pendulum: pendulums){
      pendulum.update(dt);
    }
    if(step%update_window_per_step == 0){
      // update window (drawing new mass positions)
      visualizer.update(pendulums, time);
      pendulums[0].check_energy(step, time, energy_fname);
    }
    time += dt;
  }
  std::cout << "finish simulating" << std::endl;
  return 0;
}

