#ifndef REGISTER_H
#define REGISTER_H

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Thread ID management
// ---------------------------------------------------------------------------
// Each thread has a unique ID in [0, N).
// We use a thread-local variable that each thread sets before using registers.

static _Thread_local int thread_id = -1;

static inline void set_thread_id(int id) { thread_id = id; }
static inline int get_thread_id(void) { return thread_id; }

#define MAX_THREADS 8

// ---------------------------------------------------------------------------
// Stamped value — used by atomic register constructions
// ---------------------------------------------------------------------------
typedef struct {
  long stamp;
  int value;
} StampedValue;

#define STAMPED_INIT {.stamp = 0, .value = 0}

static inline StampedValue stamped_max(StampedValue a, StampedValue b) {
  return (a.stamp > b.stamp) ? a : b;
}

static inline StampedValue stamped_max_total_order(StampedValue a,
                                                   StampedValue b,
                                                   int a_thread_id,
                                                   int b_thread_id) {
  if (a.stamp != b.stamp) {
    return stamped_max(a, b);
  }
  return (a_thread_id > b_thread_id) ? a : b;
}

#endif // REGISTER_H
