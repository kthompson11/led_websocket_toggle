// TODO: reduce scope of lock

#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

#include <iostream>
#include <system_error>
#include <stdexcept>
#include <exception>

#include "LEDDriver.h"
#include "ManagedGpiod.h"
#include "PthreadLock.h"

using nlohmann::json;

#define GPIOCHIP_PATH "/dev/gpiochip0"

/**************************** Static Thread Start Function ****************************/

void* LEDDriver::threadStart(void *arg)
{
    LEDDriver &leds = ((LEDDriverArg *)arg)->leds;
    int shutdownFD = ((LEDDriverArg *)arg)->shutdownFD;

    TimespecArithmetic nextTickTime;
    TimespecArithmetic waitTime;
    TimespecArithmetic currentTime;
    timespec zeroTS = {0};

    LEDDriverMode mode = leds.getMode();

    // setup ppoll structs
    const int nfds = 2;
    const int iShutdownPollFD = 0;
    const int iUpdatePollFD = 1;
    pollfd fds[nfds];
    // setup shutdown fd
    fds[0].fd = shutdownFD;
    fds[0].events = POLLIN;
    // setup updateFD
    fds[1].fd = leds.updateFD;
    fds[1].events = POLLIN;

    bool shutdown = false;
    while (!shutdown) {
        // wait for event or timeout
        int status;
        if (mode == LED_MODE_PATTERN) {
            nextTickTime = leds.getNextTickTime();
            if (clock_gettime(CLOCK_MONOTONIC_COARSE, currentTime.data()) == -1) {
                std::cerr << "clock_gettime() error\n";
                throw std::system_error(errno, std::generic_category());
            }
            waitTime = nextTickTime - currentTime;

            timespec ts = waitTime.getTimespec();
            if ((ts.tv_sec < 0) || (ts.tv_nsec < 0)) {
                status = ppoll(fds, nfds, &zeroTS, nullptr);
            } else {
                status = ppoll(fds, nfds, waitTime.data(), nullptr);
            }
        } else {
            status = ppoll(fds, nfds, nullptr, nullptr);
        }

        // check status and perform operation
        if (status == -1) {
            std::cerr << "LEDDriver.cpp: ppoll() error\n";
            throw std::system_error(errno, std::generic_category());
        } else if (status == 0) {
            // timeout
            leds.tick();
        } else {
            if (fds[iShutdownPollFD].revents == POLLIN) {
                // shutdown program
                shutdown = true;
            } else {
                // clear updateFD
                uint64_t discardBuf;
                read(fds[iUpdatePollFD].fd, &discardBuf, sizeof(discardBuf));
                // update state information
                mode = leds.getMode();
            }
        }
    }

    return 0;
}

/**************************** Debugging Functions ****************************/

void printState(std::vector<int> state)
{
    std::cout << "state = {";
    for (int n : state) {
        std::cout << n << ", ";
    }
    std::cout << "}\n";
}

/**************************** Helper Functions ****************************/

std::vector<int> nextStateBounce(std::vector<int> state)
{
    static bool goingRight = true;
    std::vector<int> nextState = state;
    int nPins = state.size();

    int currentPosition = 0;
    for (int i = 0; i < state.size(); ++i) {
        if (state[i] == 1) {
            currentPosition = i;
            break;
        }
    }

    nextState[currentPosition] = 0;
    if ((currentPosition == 0) && !goingRight) {
        goingRight = true;
    } else if ((currentPosition == nPins - 1) && goingRight) {
        goingRight = false;
    }

    if (goingRight) {
        nextState[currentPosition + 1] = 1;
    } else {
        nextState[currentPosition - 1] = 1;
    }

    return nextState;
}

std::vector<int> nextStateInvert(std::vector<int> state)
{
    std::vector<int> nextState(state.size(), 0);
    for (int i = 0; i < state.size(); ++i) {
        nextState[i] = (state[i] + 1) % 2;
    }

    return nextState;
}

std::vector<int> nextStateWalkRight(std::vector<int> state)
{
    std::vector<int> nextState(state.size(), 0);
    for (int i = 0; i < state.size(); ++i) {
        nextState[(i + 1) % state.size()] = state[i];
    }

    return nextState;
}

std::vector<int> nextStateWalkLeft(std::vector<int> state)
{
    std::vector<int> nextState(state.size(), 0);
    for (int i = 0; i < state.size(); ++i) {
        nextState[i] = state[(i + 1) % state.size()];
    }

    return nextState;
}

std::vector<int> getNextState(enum LEDPattern pattern, std::vector<int> state)
{
    std::vector<int> nextState;
    
    switch (pattern) {
    case LED_PATTERN_BOUNCE:
        nextState = nextStateBounce(state);
        break;
    case LED_PATTERN_INVERT:
        nextState = nextStateInvert(state);
        break;
    case LED_PATTERN_WALK_RIGHT:
        nextState = nextStateWalkRight(state);
        break;
    case LED_PATTERN_WALK_LEFT:
        nextState = nextStateWalkLeft(state);
        break;
    default:
        break;
    }

    return nextState;
}

std::vector<int> initState(enum LEDPattern pattern, int nLEDs)
{
    std::vector<int> initialState(nLEDs, 0);

    switch (pattern) {
    case LED_PATTERN_BOUNCE:
        initialState[0] = 1;
        break;
    case LED_PATTERN_INVERT:
        for (int i = 0; i < nLEDs; ++i) {
            initialState[i] = i % 2;
        }
        break;
    case LED_PATTERN_WALK_RIGHT:
        for (int i = 0; i < nLEDs; ++i) {
            initialState[i] = (i % 3) == 0 ? 1 : 0;
        }
        break;
    case LED_PATTERN_WALK_LEFT:
        for (int i = 0; i < nLEDs; ++i) {
            initialState[i] = (i % 3) == 0 ? 1 : 0;
        }
        break;
    default:
        break;
    }

    return initialState;
}

std::string getPatternString(LEDPattern pattern)
{
    std::string res;

    switch (pattern) {
    case LED_PATTERN_BOUNCE:
        res = "bounce";
        break;
    case LED_PATTERN_WALK_RIGHT:
        res = "walkRight";
        break;
    case LED_PATTERN_WALK_LEFT:
        res = "walkLeft";
        break;
    case LED_PATTERN_INVERT:
        res = "invert";
        break;
    }

    return res;
}

LEDDriverMode stringToMode(const std::string &mode)
{
    if (mode == "toggle") {
        return LED_MODE_TOGGLE;
    } else if (mode == "pattern") {
        return LED_MODE_PATTERN;
    } else {
        // TODO: throw error
        std::cerr << "stringToMode(): invalid parameter " << mode << std::endl;
    }

    // never reached
    return LED_MODE_TOGGLE;
}

LEDPattern stringToPattern(const std::string &pattern)
{
    std::string res;
    if (pattern == "bounce") {
        return LED_PATTERN_BOUNCE;
    } else if (pattern == "walkRight") {
        return LED_PATTERN_WALK_RIGHT;
    } else if (pattern == "walkLeft") {
        return LED_PATTERN_WALK_LEFT;
    } else if (pattern == "invert") {
        return LED_PATTERN_INVERT;
    } else {
        // TODO: throw error
        std::cerr << "stringToPattern: invalid parameter " << pattern << std::endl;
    }

    // never reached
    return LED_PATTERN_BOUNCE;
}

TimespecArithmetic getCurrentTime()
{
    TimespecArithmetic currentTime;
    if (clock_gettime(CLOCK_MONOTONIC_COARSE, currentTime.data()) == -1) {
        std::cerr << "clock_gettime() error\n";
        throw std::system_error(errno, std::generic_category());
    }

    return currentTime;
}

/**************************** Constructors/Destructor ****************************/

LEDDriver::LEDDriver(std::vector<unsigned int> &lineNumbers, 
                     const std::string &consumerName)
                     : chip(GPIOCHIP_PATH), lines(chip, lineNumbers)
{
    values = std::vector<int>(lines.getNLines(), 0);
    lines.requestOutput(consumerName, values);

    updateFD = eventfd(0, 0);
    if (updateFD == -1) {
        std::cerr << "Error creating updateFD\n";
        throw std::system_error(errno, std::generic_category());
    }
}

LEDDriver::~LEDDriver()
{
    pthread_mutex_destroy(&lock);
    close(updateFD);
}

/**************************** Member Functions ****************************/

json LEDDriver::getState()
{
    PthreadLock locked(&lock);
    json state;
    
    if (mode == LED_MODE_TOGGLE) {
        state["mode"] = "toggle";
        state["values"] = values;
    } else if (mode == LED_MODE_PATTERN) {
        state["mode"] = "pattern";
        state["pattern"] = getPatternString(pattern);
        state["period"] = tickPeriodMS;
    }

    return state;
}

/* Sets the LED driver mode. "mode" must be a valid driver mode. */
json LEDDriver::setMode(const std::string &mode)
{
    {
        this->mode = stringToMode(mode);

        PthreadLock locked(&lock);
        if (this->mode == LED_MODE_PATTERN) {
            TimespecArithmetic currentTime = getCurrentTime();
            nextTickTime = currentTime + tickPeriodMS;
        }
        values = initState(this->pattern, values.size());

        sendUpdateNotification();
    }

    return getState();
}

/* Sets the pattern used in pattern mode. "pattern" must be a valid pattern. */
json LEDDriver::setPattern(const std::string &pattern)
{
    PthreadLock locked(&lock);

    this->pattern = stringToPattern(pattern);
    values = initState(this->pattern, values.size());

    json res;
    res["pattern"] = getPatternString(this->pattern);

    return res;
}

json LEDDriver::setPeriod(unsigned int ms)
{
    PthreadLock locked(&lock);

    tickPeriodMS = ms;

    json res;
    res["period"] = tickPeriodMS;

    sendUpdateNotification();

    return res;
}

json LEDDriver::toggleLed(int ledNumber)
{
    PthreadLock locked(&lock);

    std::vector<int> nextValues = values;
    int value = values[ledNumber];
    value = (value + 1) % 2;
    nextValues[ledNumber] = value;

    lines.setValues(nextValues);
    values = nextValues;

    json res;
    res["values"] = values;

    return res;
}

/* Advances the state to the next state. Only works in pattern mode. 
   returns */
TimespecArithmetic LEDDriver::tick()
{
    PthreadLock locked(&lock);

    if (mode != LED_MODE_PATTERN) {
        throw std::runtime_error("tick() called when mode != LED_MODE_PATTERN");
    }

    values = getNextState(pattern, values);
    lines.setValues(values);

    nextTickTime = (getCurrentTime() + tickPeriodMS);
    
    return nextTickTime;
}

/* Notifies the LEDDriver thread that the state has been changed.
   Should only be called while the mutex is locked. */
void LEDDriver::sendUpdateNotification()
{
    uint64_t writeBuf = 1;
    write(updateFD, &writeBuf, sizeof(writeBuf));
}