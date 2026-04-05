// mrsw_regular_mval.h
// ============================================================================
// MRSW regular M-valued register FROM MRSW regular Boolean registers
// ============================================================================
//
// THE PROBLEM:
//   A safe M-valued register can return ANY value in [0, M-1] during a
//   concurrent write. If old=5 and new=10, a safe register might return 137.
//   A regular register must return one of {old, new} — values that were
//   actually written.
//
// THE TRICK: Unary (thermometer) encoding
//   Instead of storing value v as a number, keep an array of M Boolean
//   registers: r_bit[0..M-1]. Value v means r_bit[v] == true.
//
//   Reader: scan from index 0 upward, return the first index where
//           r_bit[i] is true.
//
//   Writer: to change from old value to new value, must carefully
//           set/clear bits so that a concurrent reader always finds
//           SOME true bit that corresponds to either old or new.
//
// ============================================================================

#ifndef MRSW_REGULAR_MVAL_H
#define MRSW_REGULAR_MVAL_H

#include "register.h"

#define MVAL_RANGE 256 // M — the range of values [0, M-1]

typedef struct {
  bool r_bit[MVAL_RANGE]; // array of MRSW regular Boolean registers
} MRSWRegularMValRegister;

static inline void mrsw_regular_mval_init(MRSWRegularMValRegister *reg) {
  for (int i = 0; i < MVAL_RANGE; i++) {
    reg->r_bit[i] = false;
  }
  reg->r_bit[0] = true; // initial value is 0
}

static inline int mrsw_regular_mval_read(MRSWRegularMValRegister *reg) {
  // Scan from low to high, return first true
  for (int i = 0; i < MVAL_RANGE; i++) {
    if (reg->r_bit[i]) {
      return i;
    }
  }
  // Should never reach here if register is used correctly
  return 0;
}

static inline void mrsw_regular_mval_write(MRSWRegularMValRegister *reg,
                                           int x) {
  // Set r_bit[x] to true and clear all other bits.
  reg->r_bit[x] = true;
  for (int i = x - 1; i >= 0; i--) {
    reg->r_bit[i] = false;
  }
}

#endif
