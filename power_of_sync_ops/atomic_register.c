// Atomic register attempt
//
// **Primitive:** only `atomic_load` / `atomic_store` on `_Atomic int`.
//
// **Protocol:** the obvious naïve attempt. Each thread writes its value
// to its own slot, reads the other's slot, and decides based on what it
// sees. Try several variants — none of them will work.
//
// **Expected result at N=2:** the harness detects consistency violations
// in some fraction of trials.
//
// **What this demonstrates:** Theorem 5.2.1 empirically. The registers
// carry no ordering information, so both threads can simultaneously
// conclude they went first (or last).

#include "consensus.h"

int decide(int value) {
  propose(value);
  int other = read_proposed(1 - get_thread_id());
  if (other != 0) {
    return other;
  }
  return value;
}
