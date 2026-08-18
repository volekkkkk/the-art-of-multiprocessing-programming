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
  test_retval_t err_cnt = run_consensus_test(trials);
  printf("Trials cnt: %d;\nConsistency errors: %d;\nValidity errors: %d\n",
         trials, err_cnt.consistency_err_cnt, err_cnt.validity_err_cnt);
  return EXIT_SUCCESS;
}
