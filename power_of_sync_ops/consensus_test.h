#pragma once

#include "consensus.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

typedef struct worker_args_t {
  int thread_id;
  int value;
} worker_args_t;

static inline int worker(void *arg) {
  worker_args_t *a = (worker_args_t *)arg;
  set_thread_id(a->thread_id);
  return decide(a->value);
};

static inline int test() {
  thrd_t threads[THREADS_CNT];
  worker_args_t args[THREADS_CNT];
  for (int i = 0; i < THREADS_CNT; i++) {
    args[i] = (worker_args_t){.thread_id = i, .value = i + 1000};
    if (thrd_create(&threads[i], worker, &args[i]) != thrd_success) {
      fprintf(stderr, "thrd_create failed for thread %d\n", args[i].thread_id);
      abort();
    };
  }

  int decisions[THREADS_CNT];
  for (int i = 0; i < THREADS_CNT; i++) {
    thrd_join(threads[i], &decisions[i]);
  }

  bool is_err;
  int min_val = 1000, max_val = THREADS_CNT + 999;

  for (int i = 0; i < THREADS_CNT; i++) {
    if (decisions[0] != decisions[i]) {
      is_err = true;
      break;
    }
  }

  if (decisions[0] < 1000 || decisions[0] > max_val) {
    is_err = true;
  }
  return is_err;
}

static inline int run_consensus_test(int trials) {
  int err_cnt;

  for (int i = 0; i < trials; i++) {
    err_cnt += test();
  }

  return err_cnt;
};
