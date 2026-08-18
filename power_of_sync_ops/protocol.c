#include "consensus.h"
#include "consensus_test.h"

int decide(int value) {
  propose(value);
  return read_proposed(0); /* always returns thread 0's input */
}

int main(void) { return run_consensus_test(100000); }
