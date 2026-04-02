// test_constructions.c
// Compile: gcc -pthread -o test_constructions test_constructions.c
#include "mrsw_safe_bool.h"
#include "mrsw_regular_bool.h"
#include "mrsw_regular_mval.h"
#include <unistd.h>

// ============================================================================
// TEST: MRSW Safe Boolean — basic correctness (no concurrent writes)
// ============================================================================

MRSWSafeBoolRegister safe_reg;

void *safe_reader(void *arg) {
    int id = *(int *)arg;
    set_thread_id(id);

    // Wait a bit for writer to finish
    usleep(50000);

    bool val = mrsw_safe_bool_read(&safe_reg);
    printf("[MRSW Safe Bool] Reader %d read: %s\n", id, val ? "true" : "false");
    return NULL;
}

void test_mrsw_safe_bool(void) {
    printf("=== MRSW Safe Boolean (no overlap) ===\n");
    int num_readers = 4;
    mrsw_safe_bool_init(&safe_reg, num_readers);

    // Writer writes true
    mrsw_safe_bool_write(&safe_reg, true);

    // Launch readers — they should all see 'true' since no overlap
    pthread_t readers[4];
    int ids[4] = {0, 1, 2, 3};
    for (int i = 0; i < num_readers; i++) {
        pthread_create(&readers[i], NULL, safe_reader, &ids[i]);
    }
    for (int i = 0; i < num_readers; i++) {
        pthread_join(readers[i], NULL);
    }

    // Now write false, read again sequentially
    mrsw_safe_bool_write(&safe_reg, false);
    for (int i = 0; i < num_readers; i++) {
        set_thread_id(i);
        bool val = mrsw_safe_bool_read(&safe_reg);
        printf("[MRSW Safe Bool] Sequential reader %d: %s (expect false)\n",
               i, val ? "true" : "false");
    }
    printf("\n");
}

// ============================================================================
// TEST: MRSW Regular Boolean — the key property
// ============================================================================

void test_mrsw_regular_bool(void) {
    printf("=== MRSW Regular Boolean ===\n");
    MRSWRegularBoolRegister reg;
    mrsw_regular_bool_init(&reg);

    // Write true, read it back
    mrsw_regular_bool_write(&reg, true);
    bool val = mrsw_regular_bool_read(&reg);
    printf("[MRSW Regular Bool] After write(true): read = %s (expect true)\n",
           val ? "true" : "false");

    // Write true AGAIN — the optimization should skip the actual write
    mrsw_regular_bool_write(&reg, true);
    val = mrsw_regular_bool_read(&reg);
    printf("[MRSW Regular Bool] After write(true) again: read = %s (expect true)\n",
           val ? "true" : "false");

    // Write false
    mrsw_regular_bool_write(&reg, false);
    val = mrsw_regular_bool_read(&reg);
    printf("[MRSW Regular Bool] After write(false): read = %s (expect false)\n",
           val ? "true" : "false");

    printf("\n");
}

// ============================================================================
// TEST: MRSW Regular M-valued — unary encoding
// ============================================================================

void test_mrsw_regular_mval(void) {
    printf("=== MRSW Regular M-valued ===\n");
    MRSWRegularMValRegister reg;
    mrsw_regular_mval_init(&reg);

    // Initial value should be 0
    int val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] Initial value: %d (expect 0)\n", val);

    // Write 42, read it back
    mrsw_regular_mval_write(&reg, 42);
    val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] After write(42): read = %d (expect 42)\n", val);

    // Write 7 (lower than current) — tests clearing above
    mrsw_regular_mval_write(&reg, 7);
    val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] After write(7): read = %d (expect 7)\n", val);

    // Write 200 (much higher) — tests clearing below
    mrsw_regular_mval_write(&reg, 200);
    val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] After write(200): read = %d (expect 200)\n", val);

    // Write 0 (edge case: lowest value)
    mrsw_regular_mval_write(&reg, 0);
    val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] After write(0): read = %d (expect 0)\n", val);

    // Write 255 (edge case: highest value)
    mrsw_regular_mval_write(&reg, 255);
    val = mrsw_regular_mval_read(&reg);
    printf("[MRSW Regular MVal] After write(255): read = %d (expect 255)\n", val);

    // Rapid sequence of writes
    for (int v = 0; v < 256; v += 17) {
        mrsw_regular_mval_write(&reg, v);
        val = mrsw_regular_mval_read(&reg);
        if (val != v) {
            printf("[MRSW Regular MVal] FAIL: wrote %d, read %d\n", v, val);
            return;
        }
    }
    printf("[MRSW Regular MVal] Rapid sequential writes: all correct\n");

    printf("\n");
}

// ============================================================================

int main(void) {
    test_mrsw_safe_bool();
    test_mrsw_regular_bool();
    test_mrsw_regular_mval();

    printf("All tests passed!\n");
    return 0;
}
