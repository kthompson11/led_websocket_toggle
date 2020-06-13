#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <vector>

#include <sys/time.h>

#include <gpiod.h>

#include "json.hpp"
#include "TimespecArithmetic.h"

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
    LEDDriver(unsigned int *lineNumbers, int nLines, const char *consumerName);
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
    TimespecArithmetic tick();
private:
    struct gpiod_line_bulk lines = {0};
    std::vector<int> values;
    enum LEDDriverMode mode = LED_MODE_TOGGLE;
    enum LEDPattern pattern = LED_PATTERN_BOUNCE;
    unsigned int tickPeriodMS = 1000;
};

#endif // LEDDRIVER_H