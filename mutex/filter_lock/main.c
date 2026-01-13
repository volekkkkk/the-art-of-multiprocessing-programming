#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "filter_lock.h"

volatile long counter = 0;

typedef struct {
  int id;
  int iterations;
  int threads_num;
} thread_args_t;

int increment_counter(void *arg) {
  thread_args_t *args = arg;
  for (int i = 0; i < args->iterations; i++) {
    filter_lock(args->id, args->threads_num);
    counter++;
    filter_unlock(args->id);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int iterations = 1000000;
  int threads_num = 5;

  if (argc > 1) {
    iterations = atoi(argv[1]);
    threads_num = atoi(argv[2]);
  }

  thrd_t workers[threads_num];

  counter = 0;

  for (int i = 0; i < threads_num; i++) {
    thread_args_t args_i = {i, iterations, threads_num};
    thrd_create(&workers[i], increment_counter, &args_i);
  }

  for (int i = 0; i < threads_num; i++) {
    thrd_join(workers[i], NULL);
  }

  long expected = threads_num * iterations;
  printf("counter=%ld %s\n", counter, counter == expected ? "✓" : "✗ FAILED");

  return 0;
}
