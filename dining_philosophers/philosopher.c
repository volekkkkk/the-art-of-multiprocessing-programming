#include "philosopher.h"
#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

void acquire_forks(Philosopher *philosopher) {
  if (*(&philosopher->left_fork->used_by_phil_id) != -1){
    return;
  }
  mtx_lock(&philosopher->left_fork->mtx);
  philosopher->left_fork->used_by_phil_id = philosopher->id;

  if (*(&philosopher->right_fork->used_by_phil_id) != -1){
      release_left_fork(philosopher);
      return;
  }
  mtx_lock(&philosopher->right_fork->mtx);
  philosopher->right_fork->used_by_phil_id = philosopher->id;
}

void release_left_fork(Philosopher *philosopher) {
  mtx_unlock(&philosopher->left_fork->mtx);
  philosopher->left_fork->used_by_phil_id = -1;
}

void release_right_fork(Philosopher *philosopher) {
  mtx_unlock(&philosopher->right_fork->mtx);
  philosopher->right_fork->used_by_phil_id = -1;
}

int eat(Philosopher *philosopher) {
  struct timespec eat_duration = {1, 0};
  struct timespec sleep_duration = {0, 5};

  for (;;) {
    acquire_forks(philosopher);
    philosopher->meals_count++;
    printf("Philosopher %d eats with forks %d and %d for %d time\n",
           philosopher->id, philosopher->left_fork->id,
           philosopher->right_fork->id, philosopher->meals_count);
    thrd_sleep(&eat_duration, NULL);
    release_left_fork(philosopher);
    release_right_fork(philosopher);
    thrd_sleep(&sleep_duration, NULL); // process of thinking
  }
  return 0;
}
