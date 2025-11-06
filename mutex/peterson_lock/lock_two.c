volatile int victim_;

void lock_two(int thread_id) {
    victim_ = thread_id;  // "I'm willing to wait"
    while (victim_ == thread_id) {
        // spin
    }
}

void unlock_two(int) {}
