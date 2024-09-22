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

void acquireForks(Philosopher *philosopher);
void releaseForks(Philosopher *philosopher);

int eat(Philosopher *philosopher);
