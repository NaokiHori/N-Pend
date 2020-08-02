#if !defined(PENDULUM_H)
#define PENDULUM_H

#include <vector>

class Pendulum {
  public:
    // constructor
    Pendulum(const int, const double);
    // destructor
    ~Pendulum();
    // getter
    const int get_N();
    const std::vector<double> get_thetas();
    const std::vector<double> get_lengths();
    // others
    int update(const double);
    int check_energy(void);
  private:
    // number of mass
    int N;
    // gravitational acceleration
    double g;
    // angular position
    std::vector<double> thetas;
    // angular velocity
    std::vector<double> omegas;
    // angular acceleration
    std::vector<double> alphas;
    // mass
    std::vector<double> masses;
    // length
    std::vector<double> lengths;
    // previous step angular position/velocity
    std::vector<double> thetas_old;
    std::vector<double> omegas_old;
    // matrices used to inverse
    std::vector<double> A;
    std::vector<double> B;
    std::vector<double> invA;
};

#endif // PENDULUM_H
