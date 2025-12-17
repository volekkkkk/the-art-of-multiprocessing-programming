#include "peterson_lock.h"
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

void lock_seq_cst(int thread_id) {
  lock_peterson(thread_id, memory_order_seq_cst, memory_order_seq_cst);
}
void unlock_seq_cst(int thread_id) {
  unlock_peterson(thread_id, memory_order_seq_cst);
}

void lock_acq_rel(int thread_id) {
  lock_peterson(thread_id, memory_order_release, memory_order_acquire);
}
void unlock_acq_rel(int thread_id) {
  unlock_peterson(thread_id, memory_order_release);
}

void lock_relaxed(int thread_id) {
  lock_peterson(thread_id, memory_order_relaxed, memory_order_relaxed);
}
void unlock_relaxed(int thread_id) {
  unlock_peterson(thread_id, memory_order_relaxed);
}

void lock_consume(int thread_id) {
  lock_peterson(thread_id, memory_order_release, memory_order_consume);
}
void unlock_consume(int thread_id) {
  unlock_peterson(thread_id, memory_order_release);
}

volatile long counter = 0;

typedef struct {
  const char *name;
  void (*lock)(int);
  void (*unlock)(int);
} lock_variant_t;

typedef struct {
  int id;
  int iterations;
  void (*lock)(int);
  void (*unlock)(int);
} thread_args_t;

int increment_thread(void *arg) {
  thread_args_t *args = (thread_args_t *)arg;

  for (int i = 0; i < args->iterations; i++) {
    args->lock(args->id);
    counter++;
    args->unlock(args->id);
  }
  return 0;
}

double benchmark_variant(const char *name, void (*lock)(int),
                         void (*unlock)(int), int iterations) {
  struct timespec start, end;
  thrd_t t0, t1;
  thread_args_t args0 = {0, iterations, lock, unlock};
  thread_args_t args1 = {1, iterations, lock, unlock};

  counter = 0;

  clock_gettime(CLOCK_MONOTONIC, &start);

  thrd_create(&t0, increment_thread, &args0);
  thrd_create(&t1, increment_thread, &args1);

  thrd_join(t0, NULL);
  thrd_join(t1, NULL);

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

  long expected = 2L * iterations;
  printf("%-30s: %.6f seconds, counter=%ld %s\n", name, elapsed, counter,
         counter == expected ? "✓" : "✗ FAILED");

  return elapsed;
}

int main(int argc, char *argv[]) {
  int iterations = 1000000;

  if (argc > 1) {
    iterations = atoi(argv[1]);
  }

  printf("Peterson Lock Memory Ordering Benchmark\n");
  printf("========================================\n");
  printf("Iterations per thread: %d\n", iterations);
  printf("Total expected count: %d\n\n", iterations * 2);

  lock_variant_t variants[] = {
      {"seq_cst (all operations)", lock_seq_cst, unlock_seq_cst},
      {"acquire/release", lock_acq_rel, unlock_acq_rel},
      {"consume (loads)", lock_consume, unlock_consume},
      {"relaxed (UNSAFE - for comparison)", lock_relaxed, unlock_relaxed},
  };

  int num_variants = sizeof(variants) / sizeof(variants[0]);
  double times[num_variants];

  const int runs = 3;
  printf("Running %d iterations per variant...\n\n", runs);

  for (int run = 0; run < runs; run++) {
    printf("Run %d/%d:\n", run + 1, runs);
    for (int i = 0; i < num_variants; i++) {
      double t = benchmark_variant(variants[i].name, variants[i].lock,
                                   variants[i].unlock, iterations);
      if (run == 0) {
        times[i] = t;
      } else {
        times[i] = (times[i] * run + t) / (run + 1);
      }
    }
    printf("\n");
  }

  printf("Average Times Summary:\n");
  printf("======================\n");
  double baseline = times[0];
  for (int i = 0; i < num_variants; i++) {
    double speedup = baseline / times[i];
    printf("%-30s: %.6f seconds (%.2fx vs seq_cst)\n", variants[i].name,
           times[i], speedup);
  }

  return 0;
}
