#if !defined(PENDULUM_H)
#define PENDULUM_H

#include <vector>
#include <string>

class Pendulum {
  public:
    // constructor
    Pendulum(const std::string input_file, const double g);
    // destructor
    ~Pendulum();
    // getters
    const int get_N();
    const std::vector<double> get_thetas();
    const std::vector<double> get_lengths();
    // others
    void update(const double dt);
    void check_energy(const int step, const double time, const std::string energy_fname);
  private:
    // set initial condition from file
    void read_file(const std::string input_fname);
    // number of mass
    int N;
    // gravitational acceleration
    double g;
    // mass
    std::vector<double> masses;
    // length
    std::vector<double> lengths;
    // angular position
    std::vector<double> thetas;
    // angular velocity
    std::vector<double> omegas;
    // angular acceleration
    std::vector<double> alphas;
    // previous step angular position/velocity
    std::vector<double> thetas_old;
    std::vector<double> omegas_old;
    // matrices used to inverse
    std::vector<double> A;
    std::vector<double> B;
    std::vector<double> invA;
};

#endif // PENDULUM_H
