#ifndef HANDLELEDS_H
#define HANDLELEDS_H

#include <vector>

struct HandleLEDsArg {
    int shutdownFD;  // if readable then gracefully shutdown; do not actually read
    int jobQueueFD;
    int msgQueueFD;
    std::vector<unsigned int> physLEDNumbers;
};

void* handleLEDs(void *arg);

#endif // HANDLELEDS_H