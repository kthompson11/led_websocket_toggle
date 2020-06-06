
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/select.h>

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
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        std::cout << "Could not register signal handler.\n";
        return -1;
    }

    UnixServerSocket *passiveSocket = new UnixServerSocket(SOCKET_PATH);

    Socket *clientSocket = passiveSocket->accept();

    const int nLEDs = 7;
    unsigned int physLedNumbers[nLEDs] = {7, 8, 25, 11, 9, 10, 24};
    LEDDriver leds(physLedNumbers, nLEDs, "led_ctrl");
    LEDDriverMode mode = LED_MODE_PATTERN;
    LEDPattern pattern = LED_PATTERN_BOUNCE;
    leds.setMode(mode);
    leds.setPattern(pattern);
    leds.setTickPeriodMS(10);

    timespec ts = {0};
    TimespecArithmetic nextTickTime(ts);
    TimespecArithmetic waitTime(ts);
    TimespecArithmetic currentTime(ts);
    TimespecArithmetic zeroTime(ts);

    while (!AbortProgram) {

        char buf[1024] = {0};
        clock_gettime(CLOCK_MONOTONIC_COARSE, currentTime.data());
        waitTime = nextTickTime - currentTime;

        if (waitTime.getTimespec().tv_sec < 0) {
            waitTime = zeroTime;
        }
        
        int bytesRead = clientSocket->readWithTimeout(buf, 100, waitTime.data());
        if ((bytesRead == 0) && ((ts.tv_sec != 0) || (ts.tv_nsec != 0))) {
            // socket disconnected
            AbortProgram = true;
        }

        buf[bytesRead] = '\0';
        unsigned int periodMS;
        if (bytesRead != 0) {
            nlohmann::json message = nlohmann::json::parse(buf);
            periodMS = message["periodMS"];
            leds.setTickPeriodMS(periodMS);
            std::cout << "Changing period: " << periodMS << std::endl;
        } else {
            nextTickTime = leds.tick();
        }
    }

    clientSocket->close();
    passiveSocket->close();
    delete clientSocket;
    delete passiveSocket;
    unlink("/tmp/led_ctrl");

    return 0;
}