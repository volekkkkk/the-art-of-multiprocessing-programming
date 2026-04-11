// mrmw_atomic.h
// ============================================================================
// MRMW atomic register FROM MRSW atomic registers
// ============================================================================
//
// THE PROBLEM:
//   Until now, only one thread could write. Now any thread can read or write.
//   With multiple writers, we need to order writes from different threads
//   so that all readers agree on which write "happened last."
//
// THE APPROACH:
//   Each thread i owns a MRSW atomic register a_table[i] (thread i is the
//   sole writer, all threads can read it).
//
//   Writer (thread i):
//     1. Scan ALL a_table entries to find the maximum timestamp
//     2. Write {max_stamp + 1, x} into a_table[i]
//     This ensures the new write has a strictly higher timestamp than
//     any previously visible write.
//
//   Reader:
//     1. Scan ALL a_table entries
//     2. Return the value with the highest timestamp
//     (Ties broken by thread ID — higher ID wins — to ensure total order)
//
// WHY THIS WORKS:
//   All writes are totally ordered by (timestamp, thread_id).
//   A reader always returns the "latest" value in this total order.
//   Because the underlying registers are MRSW atomic, reads of each
//   a_table[i] satisfy condition 4.1.3 — so the scan is consistent.
//
// ============================================================================

#ifndef MRMW_ATOMIC_H
#define MRMW_ATOMIC_H

#include "register.h"

typedef struct {
  StampedValue a_table[MAX_THREADS]; // one MRSW atomic register per writer
} MRMWAtomicRegister;

static inline void mrmw_atomic_init(MRMWAtomicRegister *reg, int init_val) {
  for (int i = 0; i < MAX_THREADS; i++) {
    reg->a_table[i] = (StampedValue){.stamp = 0, .value = init_val};
  }
}

static inline int mrmw_atomic_read(MRMWAtomicRegister *reg) {
  StampedValue recent = STAMPED_INIT;
  int recent_indx = 0;

  for (int i = 0; i < MAX_THREADS; i++) {
    if (reg->a_table[i].stamp > recent.stamp ||
        (reg->a_table[i].stamp == recent.stamp && i > recent_indx)) {
      recent = reg->a_table[i];
      recent_indx = i;
    }
  }
  return recent.value;
}

static inline void mrmw_atomic_write(MRMWAtomicRegister *reg, int x) {
  int me = get_thread_id();

  StampedValue recent = STAMPED_INIT;
  int recent_indx = 0;
  for (int i = 0; i < MAX_THREADS; i++) {
    if (reg->a_table[i].stamp > recent.stamp ||
        (reg->a_table[i].stamp == recent.stamp && i > recent_indx)) {
      recent = reg->a_table[i];
      recent_indx = i;
    }
  }
  reg->a_table[me] = (StampedValue){.stamp = recent.stamp + 1, .value = x};
}

#endif
