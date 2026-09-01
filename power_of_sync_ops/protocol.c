#include "consensus.h"
#include "consensus_test.h"
#include <stdio.h>
#include <stdlib.h>

int decide(int value) {
  propose(value);
  return read_proposed(0); /* always returns thread 0's input */
}

int main(void) {
  int trials = 100000;
  int err_cnt = run_consensus_test(trials);
  printf("Trials cnt: %d;\n Errors: %d\n", trials, err_cnt);
  return EXIT_SUCCESS;
}
