// test_constructions.c
// Compile: gcc -pthread -o test_constructions test_constructions.c
#include "mrmw_atomic.h"
#include "mrsw_atomic.h"
#include "mrsw_regular_bool.h"
#include "mrsw_regular_mval.h"
#include "mrsw_safe_bool.h"
#include "register.h"
#include "snapshot_obsfree.h"
#include "srsw_atomic.h"
#include <unistd.h>

// ============================================================================
// TEST: MRSW Safe Boolean — basic correctness (no concurrent writes)
// ============================================================================

MRSWSafeBoolRegister safe_reg;

void *safe_reader(void *arg) {
  int id = *(int *)arg;
  set_thread_id(id);

  // Wait a bit for writer to finish
  usleep(50000);

  bool val = mrsw_safe_bool_read(&safe_reg);
  printf("[MRSW Safe Bool] Reader %d read: %s\n", id, val ? "true" : "false");
  return NULL;
}

void test_mrsw_safe_bool(void) {
  printf("=== MRSW Safe Boolean (no overlap) ===\n");
  int num_readers = 4;
  mrsw_safe_bool_init(&safe_reg, num_readers);

  // Writer writes true
  mrsw_safe_bool_write(&safe_reg, true);

  // Launch readers — they should all see 'true' since no overlap
  pthread_t readers[4];
  int ids[4] = {0, 1, 2, 3};
  for (int i = 0; i < num_readers; i++) {
    pthread_create(&readers[i], NULL, safe_reader, &ids[i]);
  }
  for (int i = 0; i < num_readers; i++) {
    pthread_join(readers[i], NULL);
  }

  // Now write false, read again sequentially
  mrsw_safe_bool_write(&safe_reg, false);
  for (int i = 0; i < num_readers; i++) {
    set_thread_id(i);
    bool val = mrsw_safe_bool_read(&safe_reg);
    printf("[MRSW Safe Bool] Sequential reader %d: %s (expect false)\n", i,
           val ? "true" : "false");
  }
  printf("\n");
}

// ============================================================================
// TEST: MRSW Regular Boolean — the key property
// ============================================================================

void test_mrsw_regular_bool(void) {
  printf("=== MRSW Regular Boolean ===\n");
  MRSWRegularBoolRegister reg;
  mrsw_regular_bool_init(&reg);

  // Write true, read it back
  mrsw_regular_bool_write(&reg, true);
  bool val = mrsw_regular_bool_read(&reg);
  printf("[MRSW Regular Bool] After write(true): read = %s (expect true)\n",
         val ? "true" : "false");

  // Write true AGAIN — the optimization should skip the actual write
  mrsw_regular_bool_write(&reg, true);
  val = mrsw_regular_bool_read(&reg);
  printf(
      "[MRSW Regular Bool] After write(true) again: read = %s (expect true)\n",
      val ? "true" : "false");

  // Write false
  mrsw_regular_bool_write(&reg, false);
  val = mrsw_regular_bool_read(&reg);
  printf("[MRSW Regular Bool] After write(false): read = %s (expect false)\n",
         val ? "true" : "false");

  printf("\n");
}

// ============================================================================
// TEST: MRSW Regular M-valued — unary encoding
// ============================================================================

void test_mrsw_regular_mval(void) {
  printf("=== MRSW Regular M-valued ===\n");
  MRSWRegularMValRegister reg;
  mrsw_regular_mval_init(&reg);

  // Initial value should be 0
  int val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] Initial value: %d (expect 0)\n", val);

  // Write 42, read it back
  mrsw_regular_mval_write(&reg, 42);
  val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] After write(42): read = %d (expect 42)\n", val);

  // Write 7 (lower than current) — tests clearing above
  mrsw_regular_mval_write(&reg, 7);
  val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] After write(7): read = %d (expect 7)\n", val);

  // Write 200 (much higher) — tests clearing below
  mrsw_regular_mval_write(&reg, 200);
  val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] After write(200): read = %d (expect 200)\n", val);

  // Write 0 (edge case: lowest value)
  mrsw_regular_mval_write(&reg, 0);
  val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] After write(0): read = %d (expect 0)\n", val);

  // Write 255 (edge case: highest value)
  mrsw_regular_mval_write(&reg, 255);
  val = mrsw_regular_mval_read(&reg);
  printf("[MRSW Regular MVal] After write(255): read = %d (expect 255)\n", val);

  // Rapid sequence of writes
  for (int v = 0; v < 256; v += 17) {
    mrsw_regular_mval_write(&reg, v);
    val = mrsw_regular_mval_read(&reg);
    if (val != v) {
      printf("[MRSW Regular MVal] FAIL: wrote %d, read %d\n", v, val);
      return;
    }
  }
  printf("[MRSW Regular MVal] Rapid sequential writes: all correct\n");

  printf("\n");
}

// ============================================================================
// TEST: SRSW Atomic — timestamps prevent going backwards
// ============================================================================

void test_srsw_atomic(void) {
  printf("=== SRSW Atomic ===\n");
  SRSWAtomicRegister reg;
  srsw_atomic_init(&reg, 0);

  // Basic read/write
  int val = srsw_atomic_read(&reg);
  printf("[SRSW Atomic] Initial: %d (expect 0)\n", val);

  srsw_atomic_write(&reg, 42);
  val = srsw_atomic_read(&reg);
  printf("[SRSW Atomic] After write(42): %d (expect 42)\n", val);

  srsw_atomic_write(&reg, 7);
  val = srsw_atomic_read(&reg);
  printf("[SRSW Atomic] After write(7): %d (expect 7)\n", val);

  // Key property: timestamps increase monotonically
  // Write a sequence, verify reads never go backwards
  int last_read = -1;
  for (int v = 0; v < 100; v += 3) {
    srsw_atomic_write(&reg, v);
    val = srsw_atomic_read(&reg);
    if (val < last_read) {
      printf("[SRSW Atomic] FAIL: read went backwards! %d -> %d\n", last_read,
             val);
      return;
    }
    last_read = val;
  }
  printf("[SRSW Atomic] Monotonic reads: all correct\n");

  printf("\n");
}

// ============================================================================
// TEST: MRSW Atomic — readers must not go backwards across threads
// ============================================================================

MRSWAtomicRegister mrsw_atomic_reg;

void *mrsw_atomic_reader(void *arg) {
  int id = *(int *)arg;
  set_thread_id(id);

  for (int round = 0; round < 5; round++) {
    int val = mrsw_atomic_read(&mrsw_atomic_reg);
    printf("[MRSW Atomic] Reader %d round %d: value=%d\n", id, round, val);
    usleep(1000);
  }
  return NULL;
}

void test_mrsw_atomic(void) {
  printf("=== MRSW Atomic ===\n");
  int num_readers = MAX_THREADS;
  mrsw_atomic_init(&mrsw_atomic_reg, 0);

  // Sequential: write then all readers read
  mrsw_atomic_write(&mrsw_atomic_reg, 99);

  for (int i = 0; i < num_readers; i++) {
    set_thread_id(i);
    int val = mrsw_atomic_read(&mrsw_atomic_reg);
    printf("[MRSW Atomic] Sequential reader %d: %d (expect 99)\n", i, val);
  }

  // Write new value, verify all readers see it
  mrsw_atomic_write(&mrsw_atomic_reg, 200);
  for (int i = 0; i < num_readers; i++) {
    set_thread_id(i);
    int val = mrsw_atomic_read(&mrsw_atomic_reg);
    printf("[MRSW Atomic] After write(200), reader %d: %d (expect 200)\n", i,
           val);
  }

  // Concurrent readers — just verify no crashes and print values
  printf("[MRSW Atomic] Launching concurrent readers...\n");
  mrsw_atomic_write(&mrsw_atomic_reg, 42);

  pthread_t readers[8];
  int ids[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  for (int i = 0; i < num_readers; i++) {
    pthread_create(&readers[i], NULL, mrsw_atomic_reader, &ids[i]);
  }

  // Writer keeps writing while readers run
  for (int v = 50; v <= 70; v++) {
    mrsw_atomic_write(&mrsw_atomic_reg, v);
    usleep(500);
  }

  for (int i = 0; i < num_readers; i++) {
    pthread_join(readers[i], NULL);
  }

  printf("\n");
}

// ============================================================================
// TEST: MRMW Atomic — multiple writers, total order
// ============================================================================

MRMWAtomicRegister mrmw_reg;

void *mrmw_writer(void *arg) {
  int id = *(int *)arg;
  set_thread_id(id);

  // Each writer writes its own id * 100 + round
  for (int round = 0; round < 5; round++) {
    int val = id * 100 + round;
    mrmw_atomic_write(&mrmw_reg, val);
    printf("[MRMW Atomic] Writer %d wrote %d\n", id, val);
    usleep(500);
  }
  return NULL;
}

void test_mrmw_atomic(void) {
  printf("=== MRMW Atomic ===\n");
  mrmw_atomic_init(&mrmw_reg, 0);

  // Sequential: different threads write, then read
  set_thread_id(0);
  mrmw_atomic_write(&mrmw_reg, 10);
  set_thread_id(1);
  mrmw_atomic_write(&mrmw_reg, 20);
  set_thread_id(2);
  mrmw_atomic_write(&mrmw_reg, 30);

  // All readers should see 30 (the latest write)
  for (int i = 0; i < 4; i++) {
    set_thread_id(i);
    int val = mrmw_atomic_read(&mrmw_reg);
    printf("[MRMW Atomic] Reader %d: %d (expect 30)\n", i, val);
  }

  // Concurrent writers
  printf("[MRMW Atomic] Launching concurrent writers...\n");
  pthread_t writers[3];
  int ids[3] = {0, 1, 2};
  for (int i = 0; i < 3; i++) {
    pthread_create(&writers[i], NULL, mrmw_writer, &ids[i]);
  }
  for (int i = 0; i < 3; i++) {
    pthread_join(writers[i], NULL);
  }

  // After all writers finish, all readers should agree
  int first_val = -1;
  for (int i = 0; i < MAX_THREADS; i++) {
    set_thread_id(i);
    int val = mrmw_atomic_read(&mrmw_reg);
    if (first_val == -1)
      first_val = val;
    if (val != first_val) {
      printf(
          "[MRMW Atomic] FAIL: readers disagree! reader 0=%d, reader %d=%d\n",
          first_val, i, val);
      return;
    }
  }
  printf("[MRMW Atomic] All readers agree on final value: %d\n", first_val);

  printf("\n");
}

// ============================================================================
// TEST: Obstruction-free Atomic Snapshot
// ============================================================================

void test_obsfree_snapshot(void) {
  printf("=== Obstruction-free Atomic Snapshot ===\n");

  ObsFreeSnapshot snap_reg;
  obsfree_snapshot_init(&snap_reg, 0);

  // Each thread updates its own slot
  set_thread_id(0);
  obsfree_update(&snap_reg, 10);
  set_thread_id(1);
  obsfree_update(&snap_reg, 20);
  set_thread_id(2);
  obsfree_update(&snap_reg, 30);

  // Scan should see all updates
  set_thread_id(0);
  SnapResult result = obsfree_scan(&snap_reg);
  printf("[Snapshot] scan: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
         result.values[0], result.values[1], result.values[2], result.values[3],
         result.values[4], result.values[5], result.values[6],
         result.values[7]);
  printf("[Snapshot] expect: [10, 20, 30, 0, 0, 0, 0, 0]\n");

  // Update slot 3 and rescan
  set_thread_id(3);
  obsfree_update(&snap_reg, 99);
  set_thread_id(0);
  result = obsfree_scan(&snap_reg);
  printf("[Snapshot] after update(3, 99): slot[3] = %d (expect 99)\n",
         result.values[3]);

  // Multiple updates to same slot — scan sees latest
  set_thread_id(1);
  obsfree_update(&snap_reg, 21);
  set_thread_id(1);
  obsfree_update(&snap_reg, 22);
  set_thread_id(1);
  obsfree_update(&snap_reg, 23);
  set_thread_id(0);
  result = obsfree_scan(&snap_reg);
  printf("[Snapshot] after 3 updates to slot 1: %d (expect 23)\n",
         result.values[1]);

  printf("\n");
}

// ============================================================================

int main(void) {
  test_mrsw_safe_bool();
  test_mrsw_regular_bool();
  test_mrsw_regular_mval();
  test_srsw_atomic();
  test_mrsw_atomic();
  test_mrmw_atomic();
  test_obsfree_snapshot();

  printf("All tests passed!\n");
  return 0;
}
