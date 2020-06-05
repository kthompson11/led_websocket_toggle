
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <gpiod.h>

#include "UnixServerSocket.h"
#include "LEDDriver.h"
#include "TimespecArithmetic.h"

#include "json.hpp"

#define SOCKET_PATH "/tmp/led_ctrl"

static bool AbortProgram = false;

void sigint_handler(int signum)
{
  if (signum == SIGINT)
    std::cout << "Received sigint.\n";
    AbortProgram = true;
}

int main(int argc, char **argv)
{
    #ifdef COMMENTED_OUT
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        std::cout << "Could not register signal handler.\n";
        return -1;
    }

    UnixServerSocket *passiveSocket = new UnixServerSocket(SOCKET_PATH);

    std::cout << "Before accept\n";
    Socket *clientSocket = passiveSocket->accept();

    while (!AbortProgram) {
        char buf[1024] = {0};
        std::cout << "Before read\n";
        int bytesRead = clientSocket->read(buf, 100);
        std::cout << "After read\n";
        if (bytesRead > 0) {
            buf[bytesRead] = '\0';
            std::cout << buf << std::endl;
            clientSocket->write(buf, bytesRead);
        } else if (bytesRead == 0) {
            // socket disconnected
            AbortProgram = true;;
        }
    }

    clientSocket->close();
    passiveSocket->close();
    delete clientSocket;
    delete passiveSocket;
    unlink("/tmp/led_ctrl");

    return 0;
    #endif

    const int nLEDs = 7;
    unsigned int physLedNumbers[nLEDs] = {7, 8, 25, 11, 9, 10, 24};
    LEDDriver leds(physLedNumbers, nLEDs, "led_ctrl");
    leds.setMode(LED_MODE_PATTERN);
    leds.setPattern(LED_PATTERN_BOUNCE);
    leds.setTickPeriodMS(10);

    while (true) {
        TimespecArithmetic waitTime;
        waitTime = leds.tick();
        TimespecArithmetic currentTime;
        clock_gettime(CLOCK_MONOTONIC_COARSE, currentTime.data());
        waitTime = waitTime - currentTime;
        nanosleep(waitTime.data(), NULL);
    }

    std::cout << "End of main.\n";

    return 0;
}