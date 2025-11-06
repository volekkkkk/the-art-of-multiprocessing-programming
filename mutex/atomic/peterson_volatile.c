volatile int victim_volatile = 0;
volatile int flag_volatile[2] = {0, 0};

void lock_volatile(int thread_id) {
  int other = 1 - thread_id;
  flag_volatile[thread_id] = 1; // "I'm interested"
  victim_volatile = thread_id;  // "I'm willing to wait"
  while (flag_volatile[other] == 1 && victim_volatile == thread_id) {
    // spin
  }
}

void unlock_volatile(int thread_id) { flag_volatile[thread_id] = 0; }
