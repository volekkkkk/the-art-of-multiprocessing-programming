#ifndef PETERSON_LOCK_H
#define PETERSON_LOCK_H

#include <stdatomic.h>

void lock_peterson(int thread_id, memory_order store_order,
                   memory_order load_order);
void unlock_peterson(int thread_id, memory_order store_order);

#endif
