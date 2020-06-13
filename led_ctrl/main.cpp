
#include <iostream>
#include <system_error>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>

#include <gpiod.h>

#include "UnixServerSocket.h"
#include "LEDDriver.h"
#include "TimespecArithmetic.h"
#include "handleIO.h"
#include "handleLEDs.h"
#include "signalHandler.h"
#include "SigintHandler.h"
#include "Pipe.h"

#include "json.hpp"

#define SOCKET_PATH "/tmp/led_ctrl"

int main(int argc, char **argv)
{
    SigintHandler handler;
    Pipe jobQueue;
    Pipe msgQueue;

    // create thread for IO handler
    pthread_t ioThread;
    HandleIOArg ioArg;
    ioArg.shutdownFD = handler.getShutdownFD();
    ioArg.jobQueueFD = jobQueue.getWriteFD();
    ioArg.msgQueueFD = msgQueue.getReadFD();
    ioArg.socketPath = SOCKET_PATH;
    if (pthread_create(&ioThread, nullptr, handleIO, &ioArg)) {
        std::cerr << "Could not create IO thread\n";
        throw std::system_error(errno, std::generic_category());
        return -1;
    }

    // create thread for handling the LEDs
    const int nLEDs = 9;
    unsigned int physLedNumbers[nLEDs] = {7, 8, 25, 11, 9, 10, 24, 22, 23};
    HandleLEDsArg ledArg;
    ledArg.shutdownFD = handler.getShutdownFD();
    ledArg.jobQueueFD = jobQueue.getReadFD();
    ledArg.msgQueueFD = msgQueue.getWriteFD();
    ledArg.nLEDs = nLEDs;
    ledArg.physLEDNumbers = physLedNumbers;
    pthread_t ledThread;
    if (pthread_create(&ledThread, nullptr, handleLEDs, &ledArg)) {
        std::cerr << "Could not create LED thread\n";
        throw std::system_error(errno, std::generic_category());
    }

    // wait for other thread to terminate
    pthread_join(ioThread, nullptr);
    pthread_join(ledThread, nullptr);

    return 0;
}
