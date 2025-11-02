volatile int flag_[2] = {0, 0};

void lock_one(int thread_id) {
    int other = 1 - thread_id;
    flag_[thread_id] = 1;
    while (flag_[other] == 1) {
        // spin
    }
}

void unlock_one(int thread_id) {
    flag_[thread_id] = 0;
}

