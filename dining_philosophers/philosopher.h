#pragma once

#include "fork.h"
#include <threads.h>

typedef struct Philosopher {
  int id;
  Fork *left_fork;
  Fork *right_fork;
  thrd_t dining_thrd;
  int meals_count;
} Philosopher;

void acquire_forks(Philosopher *philosopher);
void release_left_fork(Philosopher *philosopher);
void release_right_fork(Philosopher *philosopher);

int eat(Philosopher *philosopher);
