#pragma once

#include "consensus.h"
#include <assert.h>
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

static inline int run_consensus_test(int trials) {
  int cnt = 0;

  thrd_t threads[THREADS_CNT];
  for (int i = 0; i < THREADS_CNT; i++) {
    worker_args_t worker_arg = {.thread_id = i, .value = i + 1000};
    if (thrd_create(&threads[i], worker, &worker_arg) != thrd_success) {
      fprintf(stderr, "thrd_create failed for thread %d\n",
              worker_arg.thread_id);
      abort();
    };
  }

  for (int i = 0; i < THREADS_CNT; i++) {
    thrd_join(threads[i], NULL);
  }
  return cnt;
};
