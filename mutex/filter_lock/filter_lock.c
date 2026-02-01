// filter_lock.c
#include <stdatomic.h>

#ifndef MAX_THREADS
#define MAX_THREADS 30
#endif

static _Atomic int level[MAX_THREADS];  // level[i] = current level of thread i
static _Atomic int victim[MAX_THREADS]; // victim[L] = victim at level L

void filter_lock(int thread_id, int n_threads) {
  for (int L = 1; L < n_threads; L++) { // n-1 levels
    atomic_store_explicit(&level[thread_id], L, memory_order_release);
    atomic_store_explicit(&victim[L], thread_id, memory_order_seq_cst);

    // Spin while conflicts exist at this level
    for (;;) {
      int conflict = 0;

      // Check if any other thread is at this level or higher
      for (int k = 0; k < n_threads; k++) {
        if (k != thread_id &&
            atomic_load_explicit(&level[k], memory_order_acquire) >= L) {
          conflict = 1;
          break;
        }
      }

      // If I'm the victim and there's a conflict, keep spinning
      if (!conflict ||
          atomic_load_explicit(&victim[L], memory_order_acquire) != thread_id) {
        break;
      }
    }
  }
}

void filter_unlock(int thread_id) {
  atomic_store_explicit(&level[thread_id], 0, memory_order_release);
}
