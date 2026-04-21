// snapshot_obsfree.h
// ============================================================================
// Obstruction-free atomic snapshot FROM MRSW atomic registers
// ============================================================================
//
// THE PROBLEM:
//   We have an array of MRSW atomic registers, one per thread.
//   Each thread writes only its own slot (update).
//   We want a scan() that returns a CONSISTENT view of all slots —
//   as if all reads happened at the same instant.
//
//   A naive loop reading slot[0], slot[1], ..., slot[N-1] is NOT atomic:
//   by the time you read slot[5], slot[0] might have changed.
//
// THE IDEA: Double collect
//   1. Do a "collect" — read all N slots, save their stamps and values.
//   2. Do another collect.
//   3. If ALL stamps match between the two collects, nothing changed
//      during the second collect — it's a consistent snapshot. Return it.
//   4. If any stamp differs, some thread updated in between. Retry from 1.
//
// WHY OBSTRUCTION-FREE (not wait-free)?
//   If updaters keep writing between your two collects, you'll never
//   get a matching pair. A single persistent updater can starve scan()
//   forever.
//
// ============================================================================

#ifndef SNAPSHOT_OBSFREE_H
#define SNAPSHOT_OBSFREE_H

#include "register.h"
#include "stdbool.h"

typedef struct {
  StampedValue a_table[MAX_THREADS];
} ObsFreeSnapshot;

// A snapshot result — values from all threads
typedef struct {
  int values[MAX_THREADS];
} SnapResult;

static inline void obsfree_snapshot_init(ObsFreeSnapshot *snap, int init_val) {
  for (int i = 0; i < MAX_THREADS; i++) {
    snap->a_table[i] = (StampedValue){.stamp = 0, .value = init_val};
  }
}

// Helper: do a single collect (non-atomic read of all slots)
static inline void obsfree_collect(ObsFreeSnapshot *snap, StampedValue *out) {
  for (int i = 0; i < MAX_THREADS; i++) {
    out[i] = snap->a_table[i];
  }
}

static inline SnapResult obsfree_scan(ObsFreeSnapshot *snap) {
  StampedValue a_old[MAX_THREADS], a_new[MAX_THREADS];

  obsfree_collect(snap, a_old);

  bool dc_success = false;
  while (!dc_success) {
    dc_success = true;
    obsfree_collect(snap, a_new);

    for (int i = 0; i < MAX_THREADS; i++) {
      if (a_old[i].stamp != a_new[i].stamp) {
        dc_success = false;
        break;
      }
    }

    if (!dc_success) {
      memcpy(a_old, a_new, sizeof(a_old));
    }
  }

  SnapResult result = {0};
  for (int i = 0; i < MAX_THREADS; i++) {
    result.values[i] = a_new[i].value;
  }

  return result;
}

static inline void obsfree_update(ObsFreeSnapshot *snap, int value) {
  int me = get_thread_id();
  long curr_stamp = snap->a_table[me].stamp;
  snap->a_table[me] = (StampedValue){.stamp = curr_stamp + 1, .value = value};
}

#endif
