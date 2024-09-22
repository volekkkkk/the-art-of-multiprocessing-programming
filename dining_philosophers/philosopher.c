#include "philosopher.h"
#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

void acquireForks(Philosopher *philosopher) {
  mtx_lock(&philosopher->left_fork->mtx);
  philosopher->left_fork->used_by_phil_id = philosopher->id;
  mtx_lock(&philosopher->right_fork->mtx);
  philosopher->right_fork->used_by_phil_id = philosopher->id;
}

void releaseForks(Philosopher *philosopher) {
  mtx_unlock(&philosopher->left_fork->mtx);
  philosopher->left_fork->used_by_phil_id = -1;
  mtx_unlock(&philosopher->right_fork->mtx);
  philosopher->right_fork->used_by_phil_id = -1;
}

int eat(Philosopher *philosopher) {
  struct timespec eat_duration = {1, 0};
  // struct timespec sleep_duration = {1};

  for (;;) {
    acquireForks(philosopher);
    philosopher->meals_count++;
    printf("Philosopher %d eats with forks %d and %d for %d time\n",
           philosopher->id, philosopher->left_fork->id,
           philosopher->right_fork->id, philosopher->meals_count);
    thrd_sleep(&eat_duration, NULL);
    releaseForks(philosopher);
    // thrd_sleep(&sleep_duration, NULL);
  }
  return 0;
}
