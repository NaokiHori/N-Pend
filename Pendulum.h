#if !defined(PENDULUM_H)
#define PENDULUM_H

typedef struct {
  int N;
  double g;
  double *theta0s;
  double *theta1s;
  double *theta2s;
  double *ms;
  double *ls;
  /* temporary */
  double *A;
  double *B;
  double *invA;
  double *theta0s_old;
  double *theta1s_old;
} pendulum_t;

extern pendulum_t *init_pendulum(const int N);
extern int update_pendulum(const double dt, pendulum_t *pendulum);
extern int check_energy(pendulum_t *pendulum);
extern int destruct_pendulum(pendulum_t *pendulum);

#endif // PENDULUM_H
