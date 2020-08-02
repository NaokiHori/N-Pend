#include "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/xlocale.h"
#include <array>
#include <chrono>
#include <thread>
#include "Pendulum.h"
#include "Visual.h"


int main(){
  const int N = 2;
  const double g = 1.;
  const double dt = 0.01;
  const int stepmax = 10000;
  /* left, top, width, height */
  const std::array<int, 4> window_position = {200, 200, 800, 800};
  visual_t *visual = init_visual(N, window_position);
  Pendulum pendulum(N, g);
  /* Visual visual(N, window_position); */
  for(int step=0; step<stepmax; step++){
    pendulum.update(dt);
    if(step%10==0){
      update_visual(visual, pendulum);
      pendulum.check_energy();
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
  }
  return 0;
}

