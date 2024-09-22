#pragma once

#include <threads.h>

typedef struct Fork {
  int id;
  mtx_t mtx;
  int used_by_phil_id;
} Fork;
