#if !defined(PENDULUM_H)
#define PENDULUM_H

#include <vector>

class Pendulum {
  public:
    Pendulum(const int);
    ~Pendulum();
    const int get_N();
    const std::vector<double> get_theta0s();
    const std::vector<double> get_ls();
    int update(const double);
    int check_energy(void);
  private:
    int N;
    double g;
    std::vector<double> theta0s;
    std::vector<double> theta1s;
    std::vector<double> theta2s;
    std::vector<double> ms;
    std::vector<double> ls;
    /* temporary */
    std::vector<double> theta0s_old;
    std::vector<double> theta1s_old;
    std::vector<double> A;
    std::vector<double> B;
    std::vector<double> invA;
};

#endif // PENDULUM_H
