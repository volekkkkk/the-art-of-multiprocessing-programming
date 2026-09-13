#pragma once

#include "consensus.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

typedef struct worker_args_t {
  int thread_id;
  int value;
} worker_args_t;

typedef struct test_retval_t {
  int consistency_err_cnt;
  int validity_err_cnt;
} test_retval_t;

static inline int worker(void *arg) {
  worker_args_t *a = (worker_args_t *)arg;
  set_thread_id(a->thread_id);
  return decide(a->value);
}

static inline test_retval_t run_trial(void) {
  thrd_t threads[THREADS_CNT];
  worker_args_t args[THREADS_CNT];

  for (int i = 0; i < THREADS_CNT; i++) {
    atomic_store_explicit(&proposed[i], 0, memory_order_relaxed);
  }

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

  test_retval_t retval = {0};
  int min_val = 1000, max_val = THREADS_CNT + 999;

  for (int i = 0; i < THREADS_CNT; i++) {
    if (decisions[0] != decisions[i]) {
      retval.consistency_err_cnt++;
      break;
    }
  }

  if (decisions[0] < min_val || decisions[0] > max_val) {
    retval.validity_err_cnt++;
  }
  return retval;
}

static inline test_retval_t run_consensus_test(int trials) {
  test_retval_t err_cnt = {0};

  for (int i = 0; i < trials; i++) {
    test_retval_t retval = run_trial();
    err_cnt.consistency_err_cnt += retval.consistency_err_cnt;
    err_cnt.validity_err_cnt += retval.validity_err_cnt;
  }

  return err_cnt;
}
