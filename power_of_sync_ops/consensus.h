#pragma once

#ifndef THREADS_CNT
#define THREADS_CNT 2
#endif

#include <stdatomic.h>
#include <threads.h>

static _Atomic int proposed[THREADS_CNT];
static thread_local int thread_id = -1;

static inline void set_thread_id(int id) { thread_id = id; }
static inline int get_thread_id() { return thread_id; }

static void propose(int v) {
  atomic_store_explicit(&proposed[get_thread_id()], v, memory_order_release);
}

static int read_proposed(int i) {
  return atomic_load_explicit(&proposed[i], memory_order_acquire);
}

int decide(int value);
