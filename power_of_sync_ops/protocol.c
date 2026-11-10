#include "consensus.h"

int decide(int value) {
  propose(value);
  return read_proposed(0); /* always returns thread 0's input */
}

// int decide(int value) {
//   propose(value);
//   return get_thread_id() + 1000; /* consistency error */
// }

// int decide(int value) {
//   propose(value);
//   return 42; /* validity error */
// }
