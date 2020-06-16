#include <system_error>
#include <iostream>

#include "PthreadLock.h"

PthreadLock::PthreadLock(pthread_mutex_t *lock)
{
    this->lock = lock;
    int status = pthread_mutex_lock(lock);

    if (status != 0) {
        std::cerr << "PthreadLock() error\n";
        throw std::system_error(status, std::generic_category());
    }
}

PthreadLock::~PthreadLock()
{
    pthread_mutex_unlock(lock);
}