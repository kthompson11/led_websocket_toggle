#ifndef HANDLELEDS_H
#define HANDLELEDS_H

struct HandleLEDsArg {
    int shutdownFD;  // if readable then gracefully shutdown; do not actually read
    int jobQueueFD;
    int msgQueueFD;
    int nLEDs;
    unsigned int *physLEDNumbers;
};

void* handleLEDs(void *arg);

#endif // HANDLELEDS_H