#ifndef HANDLEIO_H
#define HANDLEIO_H

#include <string>

struct HandleIOArg {
    int shutdownFD;  // if readable then gracefully shutdown; do not actually read
    int jobQueueFD;
    int msgQueueFD;
    std::string socketPath;
};

void* handleIO(void *arg);

#endif // HANDLEIO_H