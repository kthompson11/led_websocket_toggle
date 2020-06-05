#ifndef LEDDRIVER_H
#define LEDDRIVER_H

#include <vector>

#include <sys/time.h>

#include <gpiod.h>

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
    void setMode(enum LEDDriverMode);
    void setPattern(enum LEDPattern);
    void setTickPeriodMS(unsigned int ms);
    int toggleLed(int ledNumber);
    TimespecArithmetic tick();
private:
    struct gpiod_line_bulk lines = {0};
    std::vector<int> values;
    enum LEDDriverMode mode = LED_MODE_TOGGLE;
    enum LEDPattern pattern = LED_PATTERN_BOUNCE;
    unsigned int tickPeriodMS;
};

#endif // LEDDRIVER_H