// test_constructions.c
// Compile: gcc -pthread -o test_constructions test_constructions.c
#include "mrsw_safe_bool.h"
#include "mrsw_regular_bool.h"
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

    mrsw_regular_bool_write(&reg, false);
    val = mrsw_regular_bool_read(&reg);
    printf("[MRSW Regular Bool] After write(false): read = %s (expect false)\n",
           val ? "true" : "false");

    printf("\n");
}

// ============================================================================

int main(void) {
    test_mrsw_safe_bool();
    test_mrsw_regular_bool();

    printf("If all reads matched expectations, constructions 1 and 2 are correct!\n");
    printf("Next: MRSW regular M-valued (the unary bit trick)\n");
    return 0;
}
