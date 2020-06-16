#ifndef PTHREADLOCK_H
#define PTHREADLOCK_H

#include <pthread.h>

class PthreadLock
{
public:
    PthreadLock(pthread_mutex_t *lock);
    ~PthreadLock();
private:
    pthread_mutex_t *lock;
};

#endif // PTHREADLOCK_H