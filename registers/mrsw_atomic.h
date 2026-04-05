// mrsw_atomic.h
// ============================================================================
// MRSW atomic register FROM SRSW atomic registers
// ============================================================================
//
// THE PROBLEM:
//   Naive approach: one SRSW atomic register per reader (like construction #1).
//   Writer updates slots one by one. But:
//
//     Writer updates slot[0], gets delayed before updating slot[1].
//     Reader 0 reads new value (stamp=5).
//     Reader 1 reads AFTER reader 0 finishes — but still sees old value
//     (stamp=4) because writer hasn't reached slot[1] yet.
//     Condition 4.1.3 violated: reader 0 → reader 1, but reader 1
//     sees older value.
//
// THE FIX: n×n communication table
//   a_table[i][j] is a SRSW atomic register where reader i is the WRITER
//   and reader j is the READER. (Plus a_table[i][i] is written by the
//   actual writer and read by reader i — the diagonal.)
//
//   Writer: writes the new stamped value to all DIAGONAL entries
//           a_table[i][i] for all i, with increasing timestamp.
//
//   Reader A:
//     1. Read a_table[A][A] (its own slot from the writer)
//     2. Check its COLUMN: read a_table[B][A] for all B ≠ A
//        (see what other readers have told me)
//     3. Take the max timestamp across all of these
//     4. Write the max to its ROW: a_table[A][B] for all B ≠ A
//        (tell all other readers what I found)
//     5. Return the max value
//
//   WHY THIS WORKS:
//     If reader A saw stamp=5 and finished, it wrote stamp=5 into
//     a_table[A][B] for all B. Any later reader B will read that
//     from a_table[A][B] and see at least stamp=5.
//
// ============================================================================

#ifndef MRSW_ATOMIC_H
#define MRSW_ATOMIC_H

#include "register.h"

typedef struct {
  StampedValue a_table[MAX_THREADS][MAX_THREADS]; // n×n SRSW atomic registers
  long last_stamp;                                // writer's timestamp counter
} MRSWAtomicRegister;

static inline void mrsw_atomic_init(MRSWAtomicRegister *reg, int init_val) {
  reg->last_stamp = 0;
  StampedValue init = {.stamp = 0, .value = init_val};
  for (int i = 0; i < MAX_THREADS; i++) {
    for (int j = 0; j < MAX_THREADS; j++) {
      reg->a_table[i][j] = init;
    }
  }
}

static inline int mrsw_atomic_read(MRSWAtomicRegister *reg) {
  int me = get_thread_id();

  StampedValue r_value = reg->a_table[me][me];
  for (int i = 0; i < MAX_THREADS; i++) {
    if (i == me) {
      continue;
    }

    r_value = stamped_max(r_value, reg->a_table[i][me]);
  }

  for (int i = 0; i < MAX_THREADS; i++) {
    if (i == me) {
      continue;
    }

    reg->a_table[me][i] = r_value;
  }

  return r_value.value;
}

static inline void mrsw_atomic_write(MRSWAtomicRegister *reg, int x) {
  reg->last_stamp++;
  for (int i = 0; i < MAX_THREADS; i++) {
    reg->a_table[i][i] = (StampedValue){.stamp = reg->last_stamp, .value = x};
  }
}

#endif
