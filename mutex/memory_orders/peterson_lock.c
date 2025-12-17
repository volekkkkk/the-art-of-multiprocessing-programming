#include "peterson_lock.h"
#include <stdatomic.h>

static _Atomic int victim = 0;
static _Atomic int flag[2] = {0, 0};

void lock_peterson(int thread_id, memory_order store_order,
                   memory_order load_order) {
  int other = 1 - thread_id;

  atomic_store_explicit(&flag[thread_id], 1, store_order);
  atomic_store_explicit(&victim, thread_id, store_order);

  while (atomic_load_explicit(&flag[other], load_order) == 1 &&
         atomic_load_explicit(&victim, load_order) == thread_id) {
  }
}

void unlock_peterson(int thread_id, memory_order store_order) {
  atomic_store_explicit(&flag[thread_id], 0, store_order);
}
