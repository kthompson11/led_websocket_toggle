#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <vector>

#include <sys/time.h>

#include <gpiod.h>

#include "json.hpp"
#include "TimespecArithmetic.h"
#include "ManagedGpiod.h"

enum LEDDriverMode {
    LED_MODE_TOGGLE,
    LED_MODE_PATTERN
};

enum LEDPattern {
    LED_PATTERN_BOUNCE,
    LED_PATTERN_WALK_RIGHT,
    LED_PATTERN_WALK_LEFT,
    LED_PATTERN_INVERT
};

class LEDDriver
{
public:
    LEDDriver(std::vector<unsigned int> &lineNumbers, const std::string &consumerName);
    ~LEDDriver();
    nlohmann::json getState();
    nlohmann::json setMode(const std::string &mode);
    nlohmann::json setPattern(const std::string &pattern);
    nlohmann::json setPeriod(unsigned int ms);
    nlohmann::json toggleLed(int ledNumber);
    std::vector<int> getValues() { return values; }
    LEDDriverMode getMode() { return mode; }
    LEDPattern getPattern() { return pattern; }
    unsigned int getPeriod() { return tickPeriodMS; }
    TimespecArithmetic getNextTickTime() { return nextTickTime; }
    TimespecArithmetic tick();
    static void* threadStart(void *arg);

    // deleted methods
    LEDDriver(const LEDDriver&) = delete;
    LEDDriver& operator=(const LEDDriver&) = delete;
    LEDDriver(const LEDDriver&&) = delete;
    LEDDriver& operator=(const LEDDriver&&) = delete;
private:
    void sendUpdateNotification();

    std::vector<int> values;
    enum LEDDriverMode mode = LED_MODE_TOGGLE;
    enum LEDPattern pattern = LED_PATTERN_BOUNCE;
    unsigned int tickPeriodMS = 1000;
    TimespecArithmetic nextTickTime;

    int updateFD;
    GPIOD::Chip chip;
    GPIOD::LineBulk lines;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
};

struct LEDDriverArg {
    int shutdownFD;
    LEDDriver &leds;
};

#endif // LEDDRIVER_H