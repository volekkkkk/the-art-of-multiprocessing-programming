#include "fork.h"
#include "philosopher.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#ifndef SEATS_CNT
#define SEATS_CNT 5
#endif

Philosopher *philosophers[SEATS_CNT];
Fork *forks[SEATS_CNT];

void handle_sigint(int sig) {
  printf("\nCaught signal %d, cleaning up and exiting...\n", sig);

  for (int i = 0; i < SEATS_CNT; i++) {
    if (philosophers[i] != NULL) {
      printf("Philosopher %d ate %d times\n", philosophers[i]->id,
             philosophers[i]->meals_count);
      free(philosophers[i]);
    }
    if (forks[i] != NULL) {
      free(forks[i]);
    }
  }
  exit(0);
}

int init_forks(Fork *forks[]) {
  for (int i = 0; i < SEATS_CNT; i++) {
    Fork *fork = malloc(sizeof(Fork));
    if (fork == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      return 1;
    }
    forks[i] = fork;
    fork->id = i;
  }
  return 0;
}

int init_philosophers(Philosopher *philosophers[], Fork *forks[]) {
  for (int i = 0; i < SEATS_CNT; i++) {
    Philosopher *philo = malloc(sizeof(Philosopher));
    if (philo == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      return 1;
    }
    philosophers[i] = philo;
    philo->id = i;
    philo->left_fork = forks[i];
    philo->right_fork = (i == SEATS_CNT - 1) ? forks[0] : forks[i + 1];
    thrd_create(&philo->dining_thrd, (thrd_start_t)eat, philo);
  }
  return 0;
}

int main() {
  signal(SIGINT, handle_sigint);

  init_forks(forks);
  init_philosophers(philosophers, forks);

  for (int i = 0; i < SEATS_CNT; i++) {
    thrd_join(philosophers[i]->dining_thrd, NULL);
  }

  handle_sigint(SIGINT);
  return 0;
}
