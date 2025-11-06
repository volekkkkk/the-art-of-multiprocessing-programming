#include <stdio.h>
#include <threads.h>

#include "peterson_atomic.h"
#include "peterson_plain.h"
#include "peterson_volatile.h"

volatile long counter = 0;

int increment_broken(void *) {
  for (int i = 0; i < 1000000; i++) {
    counter++;
  }
  return 0;
}

int increment_plain(void *arg) {
  int id = *(int *)arg;
  for (int i = 0; i < 1000000; i++) {
    lock_plain(id);
    counter++;
    unlock_plain(id);
  }
  return 0;
}

int increment_volatile(void *arg) {
  int id = *(int *)arg;
  for (int i = 0; i < 1000000; i++) {
    lock_volatile(id);
    counter++;
    unlock_volatile(id);
  }
  return 0;
}

int increment_atomic(void *arg) {
  int id = *(int *)arg;
  for (int i = 0; i < 1000000; i++) {
    lock_atomic(id);
    counter++;
    unlock_atomic(id);
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

  // Plain Peterson lock version - does not work with -O2 flag
  // counter = 0;
  // thrd_create(&t0, (thrd_start_t)increment_plain, &id0);
  // thrd_create(&t1, (thrd_start_t)increment_plain, &id1);
  // thrd_join(t0, NULL);
  // thrd_join(t1, NULL);
  // printf("Peterson lock version: %ld (expected 2000000)\n", counter);

  // Volatile lock version
  counter = 0;
  thrd_create(&t0, (thrd_start_t)increment_volatile, &id0);
  thrd_create(&t1, (thrd_start_t)increment_volatile, &id1);
  thrd_join(t0, NULL);
  thrd_join(t1, NULL);
  printf("Peterson lock version with volatile: %ld (expected 2000000)\n",
         counter);

  // Atomic lock version
  counter = 0;
  thrd_create(&t0, (thrd_start_t)increment_atomic, &id0);
  thrd_create(&t1, (thrd_start_t)increment_atomic, &id1);
  thrd_join(t0, NULL);
  thrd_join(t1, NULL);
  printf("Peterson lock version with atomics: %ld (expected 2000000)\n",
         counter);
  return 0;
}
