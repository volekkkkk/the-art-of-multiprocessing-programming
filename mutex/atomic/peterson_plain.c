int victim = 0;
int flag[2] = {0, 0};

void lock_plain(int thread_id) {
  int other = 1 - thread_id;
  flag[thread_id] = 1; // "I'm interested"
  victim = thread_id;  // "I'm willing to wait"
  while (flag[other] == 1 && victim == thread_id) {
    // spin
  }
}

void unlock_plain(int thread_id) { flag[thread_id] = 0; }
