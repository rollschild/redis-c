#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <cstddef>
#include <deque>
#include <pthread.h>
#include <vector>
struct Work {
    void (*f)(void *) = nullptr;
    void *arg = nullptr;
};

struct ThreadPool {
    std::vector<pthread_t> threads;
    std::deque<Work> queue;
    pthread_mutex_t mu;
    pthread_cond_t is_not_empty;
};

void thread_pool_init(ThreadPool *tp, size_t num_of_threads);
void *worker(void *arg);
void thread_pool_queue(ThreadPool *tp, void (*f)(void *), void *arg);

#endif /* THREAD_POOL_H */
