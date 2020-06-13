#include "LEDDriver.h"

#include <iostream>
#include <system_error>

using nlohmann::json;

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

json LEDDriver::getState()
{
    json state;
    
    std::string s;
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
    this->mode = stringToMode(mode);
    values = initState(this->pattern, values.size());

    return getState();
}

/* Sets the pattern used in pattern mode. "pattern" must be a valid pattern. */
json LEDDriver::setPattern(const std::string &pattern)
{
    this->pattern = stringToPattern(pattern);
    values = initState(this->pattern, values.size());

    json res;
    res["pattern"] = getPatternString(this->pattern);

    return res;
}

json LEDDriver::setPeriod(unsigned int ms)
{
    tickPeriodMS = ms;

    json res;
    res["period"] = tickPeriodMS;

    return res;
}

json LEDDriver::toggleLed(int ledNumber)
{
    std::vector<int> nextValues = values;
    int value = values[ledNumber];
    value = (value + 1) % 2;
    nextValues[ledNumber] = value;

    int status = gpiod_line_set_value_bulk(&lines, nextValues.data());
    if (status == 0) {
        values = nextValues;
    }

    json res;
    res["values"] = values;

    return res;
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