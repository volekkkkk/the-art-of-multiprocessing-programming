#include <threads.h>
#include <stdio.h>

#include "lock_one.h"
#include "lock_two.h"
#include "peterson.h"

volatile long counter = 0;

int increment_broken(void*) {
    for (int i = 0; i < 1000000; i++) {
        counter++;
    }
    return 0;
}

int increment_lockone(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1000000; i++) {
        // printf("incrementing %d\n", id);
        lock_one(id);
        counter++;
        unlock_one(id);
    }
    return 0;
}

int increment_locktwo(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1000000; i++) {
        printf("Thread %d: Setting victim=%d\n", id, id);
        lock_two(id);
        counter++;
        unlock_two(id);
        printf("Counter=%ld\n", counter);
    }
    return 0;
}

int increment_peterson(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1000000; i++) {
        lock(id);
        counter++;
        unlock(id);
    }
    return 0;
}



int main() {
    thrd_t t0, t1;
    int id0 = 0, id1 = 1;
    
    // Broken version
    counter = 0;
    thrd_create(&t0, (thrd_start_t)increment_broken, NULL);
    thrd_create(&t1, (thrd_start_t)increment_broken, NULL);
    thrd_join(t0, NULL);
    thrd_join(t1, NULL);
    printf("Broken version: %ld (expected 2000000)\n", counter);
    
    // LockOne version
    // counter = 0;
    // thrd_create(&t0, (thrd_start_t)increment_lockone, &id0);
    // thrd_create(&t1, (thrd_start_t)increment_lockone, &id1);
    // thrd_join(t0, NULL);
    // thrd_join(t1, NULL);
    // printf("LockOne version: %ld (expected 2000000)\n", counter);
    
    // LockTwo version
    // counter = 0;
    // thrd_create(&t0, (thrd_start_t)increment_locktwo, &id0);
    // thrd_create(&t1, (thrd_start_t)increment_locktwo, &id1);
    // thrd_join(t0, NULL);
    // thrd_join(t1, NULL);
    // printf("LockTwo version: %ld (expected 2000000)\n", counter);

    // Peterson lock version
    counter = 0;
    thrd_create(&t0, (thrd_start_t)increment_peterson, &id0);
    thrd_create(&t1, (thrd_start_t)increment_peterson, &id1);
    thrd_join(t0, NULL);
    thrd_join(t1, NULL);
    printf("Peterson lock version: %ld (expected 2000000)\n", counter);

    return 0;
}
