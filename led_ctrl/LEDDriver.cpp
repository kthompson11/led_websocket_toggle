#include "LEDDriver.h"

#include <iostream>
#include <system_error>

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

std::vector<int> getNextState(enum LEDPattern pattern, std::vector<int> state)
{
    std::vector<int> nextState;
    
    switch (pattern) {
    case LED_PATTERN_BOUNCE:
        nextState = nextStateBounce(state);
        break;
    case LED_PATTERN_INVERT:
        break;
    case LED_PATTERN_WALK_RIGHT:
        break;
    case LED_PATTERN_WALK_LEFT:
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
        break;
    case LED_PATTERN_WALK_RIGHT:
        break;
    case LED_PATTERN_WALK_LEFT:
        break;
    default:
        break;
    }

    return initialState;
}

/**************************** Constructors/Destructor ****************************/

LEDDriver::LEDDriver(unsigned int *lineNumbers, int nLines, const char *consumerName)
{
    for (int i = 0; i < nLines; ++i) {
        values.push_back(0);
    }

    struct gpiod_chip *chip;
	chip = gpiod_chip_open("/dev/gpiochip0");

    // store lines in struct gpiod_line_bulk object
    gpiod_chip_get_lines(chip, lineNumbers, nLines, &lines);

    // request the lines
    gpiod_line_request_bulk_output(&lines, consumerName, values.data());
}

LEDDriver::~LEDDriver()
{
    gpiod_line_release_bulk(&lines);
}

/**************************** Member Functions ****************************/

/* Sets the LED driver mode. "mode" must be a valid driver mode. */
void LEDDriver::setMode(enum LEDDriverMode mode)
{
    this->mode = mode;
    values = initState(pattern, values.size());
}

/* Sets the pattern used in pattern mode. "pattern" must be a valid pattern. */
void LEDDriver::setPattern(enum LEDPattern pattern)
{
    this->pattern = pattern;
    values = initState(pattern, values.size());
}

void LEDDriver::setTickPeriodMS(unsigned int ms)
{
    tickPeriodMS = ms;
}

int LEDDriver::toggleLed(int ledNumber)
{
    std::vector<int> nextValues = values;
    int value = values[ledNumber];
    value = (value + 1) % 2;
    nextValues[ledNumber] = value;

    int status = gpiod_line_set_value_bulk(&lines, nextValues.data());
    if (status == 0) {
        values = nextValues;
    }

    return status;
}

/* Advances the state to the next state. Only works in pattern mode. 
   returns */
TimespecArithmetic LEDDriver::tick()
{
    TimespecArithmetic nextTickTime;
    if (clock_gettime(CLOCK_MONOTONIC_COARSE, nextTickTime.data()) == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    if (mode == LED_MODE_PATTERN) {
        values = getNextState(pattern, values);
        int status = gpiod_line_set_value_bulk(&lines, values.data());
    }

    TimespecArithmetic result;
    result = (nextTickTime + tickPeriodMS);
    
    return result;
}