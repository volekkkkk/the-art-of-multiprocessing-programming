// mrsw_safe_bool.h
// ============================================================================
// MRSW safe Boolean register FROM SRSW safe Boolean registers
// ============================================================================
//
// IDEA: The writer keeps a separate SRSW register for each reader.
//       On write: update ALL copies.
//       On read:  each reader reads only its own copy.
//
// WHY IS THIS SAFE (and not regular)?
//   - If read doesn't overlap write → reader gets correct value
//   - If read DOES overlap write → the reader's SRSW register is being
//     written, so it could return any value in range
//
// EXERCISE: Fill in read() and write()
// ============================================================================

#ifndef MRSW_SAFE_BOOL_H
#define MRSW_SAFE_BOOL_H

#include "register.h"

typedef struct {
    bool s_table[MAX_THREADS]; // one SRSW safe Boolean register per reader
    int  num_readers;
} MRSWSafeBoolRegister;

static inline void mrsw_safe_bool_init(MRSWSafeBoolRegister *reg, int num_readers) {
    reg->num_readers = num_readers;
    for (int i = 0; i < num_readers; i++) {
        reg->s_table[i] = false;
    }
}

static inline bool mrsw_safe_bool_read(MRSWSafeBoolRegister *reg) {
	int thread_id = get_thread_id();
	bool res = reg->s_table[thread_id];
    return res;
}

static inline void mrsw_safe_bool_write(MRSWSafeBoolRegister *reg, bool x) {
	for (int i = 0; i < reg->num_readers; i++) {
		reg->s_table[i] = x;
	}
}

#endif
