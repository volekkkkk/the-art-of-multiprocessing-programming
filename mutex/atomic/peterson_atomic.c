#include <stdatomic.h>

_Atomic int victim_atomic = 0;
_Atomic int flag_atomic[2] = {0, 0};

void lock_atomic(int thread_id) {
  int other = 1 - thread_id;

  atomic_store_explicit(&flag_atomic[thread_id], 1,
                        memory_order_seq_cst); // "I'm interested"
  atomic_store_explicit(&victim_atomic, thread_id,
                        memory_order_seq_cst); // "I'm willing to wait"

  while (atomic_load_explicit(&flag_atomic[other], memory_order_seq_cst) == 1 &&
         atomic_load_explicit(&victim_atomic, memory_order_seq_cst) ==
             thread_id) {
    // spin
  }
}

void unlock_atomic(int thread_id) {
  atomic_store_explicit(&flag_atomic[thread_id], 0, memory_order_seq_cst);
}
