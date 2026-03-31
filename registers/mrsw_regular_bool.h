// mrsw_regular_bool.h
// ============================================================================
// MRSW regular Boolean register FROM MRSW safe Boolean register
// ============================================================================
//
// THE PROBLEM WITH SAFE:
//   Imagine the register holds 'true', and the writer writes 'true' again.
//   A safe register might FLICKER during the write — a concurrent reader
//   could see 'false' even though both old and new values are 'true'!
//   A regular register must return one of {old_value, new_value}.
//   When old == new == true, that means it MUST return true.
//
// THE FIX:
//   The writer remembers the last value it wrote.
//   If the new value == last value, DON'T WRITE (skip it).
//   This way the underlying safe register is never disturbed when
//   the value isn't actually changing, so no flicker can occur.
//
// WHY DOES THIS MAKE IT REGULAR?
//   - If read doesn't overlap write → correct value (inherited from safe)
//   - If read overlaps write AND old != new → for Boolean, {old,new} = {0,1},
//     which is the entire range, so ANY value the safe register returns
//     is valid. Regular condition satisfied!
//   - If old == new → we don't write, so no overlap happens.
//
// ============================================================================

#ifndef MRSW_REGULAR_BOOL_H
#define MRSW_REGULAR_BOOL_H

#include "register.h"

typedef struct {
    bool s_value;           // underlying MRSW safe Boolean register
    bool last_written;      // writer's memory of last value written
} MRSWRegularBoolRegister;

static inline void mrsw_regular_bool_init(MRSWRegularBoolRegister *reg) {
    reg->s_value = false;
    reg->last_written = false;
}

static inline bool mrsw_regular_bool_read(MRSWRegularBoolRegister *reg) {
    // Straightforward — just read the underlying register
    return reg->s_value;
}

static inline void mrsw_regular_bool_write(MRSWRegularBoolRegister *reg, bool x) {
    // Only write to s_value if x differs from last_written
	if (x != reg->last_written){
		reg->last_written = x;
		reg->s_value = x;
	}
}

#endif
