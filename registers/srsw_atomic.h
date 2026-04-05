// srsw_atomic.h
// ============================================================================
// SRSW atomic register FROM MRSW regular register
// ============================================================================
//
// THE GAP BETWEEN REGULAR AND ATOMIC:
//   Regular: if two reads overlap the same write, they can independently
//   return old or new. So read A might return new_value, and a LATER
//   read B might return old_value. Time goes "backwards."
//
//   Atomic adds condition 4.1.3: if read A → read B (A finishes before
//   B starts), and A returned value from write Wi, then B must return
//   a value from Wi or later. No going back.
//
// THE FIX:
//   Writer: tags each value with an incrementing timestamp.
//   Reader: remembers the highest-stamped value it ever read.
//           On each read, it compares the register's current value
//           with its remembered value, and returns whichever has
//           the higher timestamp.
//
// WHY THIS WORKS:
//   If read A saw stamp=5, then read B (which starts after A finishes)
//   will also remember stamp=5 (or higher). Even if the underlying
//   regular register "flickers" back to stamp=4 during a concurrent
//   write, the reader's memory prevents going backwards.
//
// NOTE: This is SRSW — only one reader, one writer. The reader's
//       "last read" memory is private (no sharing between readers).
//       Construction #5 extends this to MRSW.
//
// EXERCISE: Fill in read() and write()
// ============================================================================

#ifndef SRSW_ATOMIC_H
#define SRSW_ATOMIC_H

#include "register.h"

typedef struct {
    StampedValue r_value;       // underlying MRSW regular register (stamped)
    StampedValue last_read;     // reader's memory of highest-stamped value seen
    long         last_stamp;    // writer's last used timestamp
} SRSWAtomicRegister;

static inline void srsw_atomic_init(SRSWAtomicRegister *reg, int init_val) {
    reg->r_value   = (StampedValue){ .stamp = 0, .value = init_val };
    reg->last_read = (StampedValue){ .stamp = 0, .value = init_val };
    reg->last_stamp = 0;
}

static inline int srsw_atomic_read(SRSWAtomicRegister *reg) {
	StampedValue r_value = stamped_max(reg->r_value, reg->last_read);
	reg->last_read = r_value;
    return r_value.value;
}

static inline void srsw_atomic_write(SRSWAtomicRegister *reg, int x) {
	reg->last_stamp++;
	reg->r_value = (StampedValue){ .stamp=reg->last_stamp, .value = x};
}

#endif
