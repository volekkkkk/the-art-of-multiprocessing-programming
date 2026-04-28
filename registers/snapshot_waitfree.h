// snapshot_waitfree.h
// ============================================================================
// Wait-free atomic snapshot
// ============================================================================
//
// THE PROBLEM WITH OBSTRUCTION-FREE:
//   If updaters keep writing between your two collects, scan() retries
//   forever. Not wait-free.
//
// THE FIX: Embedded snapshots + pigeonhole
//   Each updater, before writing its value, takes a scan() and embeds
//   the snapshot result in its register slot. Now each slot contains:
//     { stamp, value, embedded_snapshot }
//
//   Scanner algorithm:
//     1. Do repeated collects.
//     2. If two consecutive collects match (all stamps equal) — done,
//        return the values (same as obstruction-free).
//     3. Track how many times each thread's stamp has changed.
//        If any thread moved TWICE, that thread completed a full
//        update() between two of our collects. Its embedded snapshot
//        was taken during our scan — steal it and return it.
//
//   WHY TWICE (not once)?
//     If thread T moved once, it MIGHT have started its update before
//     our scan began — its embedded snapshot could be stale.
//     But if T moved twice, the second update started AFTER our scan
//     began (it couldn't have been in progress when we started, because
//     we already saw it finish once). So the second update's embedded
//     snapshot is guaranteed to fall within our scan interval.
//
//   WHY WAIT-FREE?
//     After at most N+1 collects (where N = number of threads), by
//     pigeonhole at least one thread must have moved twice. So scan()
//     always terminates in a bounded number of steps.
//
// ============================================================================

#ifndef SNAPSHOT_WAITFREE_H
#define SNAPSHOT_WAITFREE_H

#include "register.h"

typedef struct {
  int values[MAX_THREADS];
} WFSnapResult;

typedef struct {
  long stamp;
  int value;
  WFSnapResult snap; // snapshot taken by this updater
} WFStampedValue;

typedef struct {
  WFStampedValue a_table[MAX_THREADS];
} WaitFreeSnapshot;

static inline void wf_snapshot_init(WaitFreeSnapshot *wf, int init_val) {
  WFSnapResult empty = {0};
  for (int i = 0; i < MAX_THREADS; i++) {
    // Set initial value in the embedded snapshot too
    for (int j = 0; j < MAX_THREADS; j++) {
      empty.values[j] = init_val;
    }
    wf->a_table[i] =
        (WFStampedValue){.stamp = 0, .value = init_val, .snap = empty};
  }
}

static inline void wf_collect(WaitFreeSnapshot *wf, WFStampedValue *out) {
  for (int i = 0; i < MAX_THREADS; i++) {
    out[i] = wf->a_table[i];
  }
}

static inline WFSnapResult wf_scan(WaitFreeSnapshot *snap) {
  // TODO:
  //   1. Keep a "moved" counter per thread, initialized to 0.
  //   2. Do an initial collect into 'old'.
  //   3. Loop:
  //      a. Collect into 'cur'.
  //      b. If all stamps match (old[i].stamp == cur[i].stamp for all i),
  //         the double-collect succeeded — build result from cur values.
  //      c. For each slot where stamp changed:
  //         - Increment moved[i]
  //         - If moved[i] >= 2, thread i moved twice — return its
  //           embedded snapshot: cur[i].snap
  //      d. Copy cur into old (memcpy), continue loop.

  WFStampedValue a_old[MAX_THREADS], a_new[MAX_THREADS];
  bool moved[MAX_THREADS] = {false};

  wf_collect(snap, a_old);

  bool dc_success = false;
  while (!dc_success) {
    dc_success = true;
    wf_collect(snap, a_new);

    for (int i = 0; i < MAX_THREADS; i++) {
      if (a_old[i].stamp != a_new[i].stamp) {
        if (moved[i]) {
          return a_new[i].snap;
        } else {
          moved[i] = true;
          dc_success = false;
        }
      }
    }

    if (!dc_success) {
      memcpy(a_old, a_new, sizeof(a_old));
    }
  }

  WFSnapResult result = {0};
  for (int i = 0; i < MAX_THREADS; i++) {
    result.values[i] = a_new[i].value;
  }

  return result;
}

static inline void wf_update(WaitFreeSnapshot *snap, int value) {
  int me = get_thread_id();
  WFSnapResult curr_snap = wf_scan(snap);
  long stamp = snap->a_table[me].stamp;
  snap->a_table[me] =
      (WFStampedValue){.stamp = stamp + 1, .value = value, .snap = curr_snap};
}

#endif
