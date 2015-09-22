#include <iostream>
#include <random>
#include <cstdint>
#include <chrono>

using Clock = std::conditional<std::chrono::high_resolution_clock::is_steady,
                               std::chrono::high_resolution_clock,
                               std::chrono::steady_clock>::type;

using index_t = std::int_fast32_t;
const index_t N = 1'000'000;

using Scalar = double;
using Duration = std::chrono::nanoseconds;

template <class T>
void do_not_optimize_away(T &&x) {
  asm volatile("" : "+r"(x));
}

void compute(const double* x, index_t N, double* y) {
#pragma vector nontemporal
  for(index_t i=0; i<N; ++i)
    y[i] = x[i]*x[i];
}

void random_fill(Scalar *x) {
  std::mt19937 rng{std::random_device()()};
  std::uniform_real_distribution<Scalar> dist(-1, 1);
  for (index_t i = 0; i < N; ++i) x[i] = dist(rng);
}

int get_delta(Clock::time_point time0, Clock::time_point time1) {
  return std::chrono::duration_cast<Duration>(time1 - time0).count();
}

int section() {
  Scalar *x = new Scalar[N];
  std::fill_n(x, N, 0);
  random_fill(x);
  Scalar *y = new Scalar[N];

  std::fill_n(y, N, 0);

  auto time0 = Clock::now();
  compute(x, N, y);
  auto time1 = Clock::now();

  do_not_optimize_away(y[0]);

  delete[] x;
  delete[] y;

  return get_delta(time0, time1);
}

int main() {
  double time=0;
  int num_trials = 100;
  for (int i = 0; i < num_trials; ++i) {
    time += section();
  }
  time /= num_trials;
  std::cout << "duration:: " << time << "\n";
  return 0;
}
