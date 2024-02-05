#include "thread_pool.h"
#include <cassert>
#include <cstddef>
#include <pthread.h>

/**
 * Consumer
 */
void *worker(void *arg) {
    ThreadPool *tp = (ThreadPool *)arg;
    while (true) {
        pthread_mutex_lock(&(tp->mu));
        // wait for the condition - a non-empty queue
        while (tp->queue.empty()) {
            // go to sleep, but can be awaken by pthread_cond_wait
            // the `pthread_cond_wait` function is _ALWAYS_ inside a loop,
            // checking for <condition>
            // this function _blocks_ on the `is_not_empty` variable until it
            // becomes true
            pthread_cond_wait(&(tp->is_not_empty),
                              &tp->mu); // notice the mutex here!
            /* upon successful return of the `pthread_cond_wait` function,
             * the mutex is locked and owned by the calling thread
             */
        }

        // got the job
        Work w = tp->queue.front(); // grab the job from queue
        tp->queue.pop_front();
        pthread_mutex_unlock(&(tp->mu)); // release the mutex

        // do the work
        w.f(w.arg);
    }

    return nullptr;
}

/**
 * Producer
 */
void thread_pool_queue(ThreadPool *tp, void (*f)(void *), void *arg) {
    Work w;
    w.f = f;
    w.arg = arg;

    // only one thread can access the thread at a time
    pthread_mutex_lock(&(tp->mu));
    tp->queue.push_back(w);
    // wake up a potentially sleeping consumer
    // does _not_ need to be protected by the mutex;
    // signaling after releasing the mutex is also correct
    pthread_cond_signal(&(tp->is_not_empty));
    pthread_mutex_unlock(&(tp->mu));
}

/**
 * Initialize and start threads
 */
void thread_pool_init(ThreadPool *tp, size_t num_of_threads) {
    assert(num_of_threads > 0);

    int rv = pthread_mutex_init(&(tp->mu), nullptr);
    assert(rv == 0);
    rv = pthread_cond_init(&(tp->is_not_empty), nullptr);
    assert(rv == 0);

    tp->threads.resize(num_of_threads);
    for (size_t i = 0; i < num_of_threads; ++i) {
        int rv = pthread_create(&(tp->threads[i]), nullptr, &worker, tp);
    }
}
