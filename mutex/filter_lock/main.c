#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

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

double benchmark_threads(int threads_num, int iterations) {
  thrd_t workers[threads_num];
  thread_args_t args[threads_num];
  struct timespec start, end;

  counter = 0;

  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < threads_num; i++) {
    args[i].id = i;
    args[i].iterations = iterations;
    args[i].threads_num = threads_num;
    thrd_create(&workers[i], increment_counter, &args[i]);
  }

  for (int i = 0; i < threads_num; i++) {
    thrd_join(workers[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  long expected = (long)threads_num * iterations;
  if (counter != expected) {
    printf("ERROR: counter=%ld, expected=%ld\n", counter, expected);
    exit(1);
  }

  double elapsed = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;
  return elapsed;
}

int main(int argc, char *argv[]) {
  int iterations = 1000000;
  int thread_counts[] = {2, 3, 5, 8, 10};
  int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);

  if (argc > 1) {
    iterations = atoi(argv[1]);
  }

  printf("Filter Lock Benchmark\n");
  printf("=====================\n");
  printf("Iterations per thread: %d\n\n", iterations);
  printf("%-10s %-15s %-20s %-20s\n", "Threads", "Time (s)", "Total Ops", "Ops/sec");
  printf("------------------------------------------------------------------------\n");

  for (int i = 0; i < num_tests; i++) {
    int threads = thread_counts[i];
    double elapsed = benchmark_threads(threads, iterations);
    long total_ops = (long)threads * iterations;
    double ops_per_sec = total_ops / elapsed;

    printf("%-10d %-15.6f %-20ld %-20.0f\n",
           threads, elapsed, total_ops, ops_per_sec);
  }

  printf("\nAll tests passed ✓\n");
  return 0;
}
