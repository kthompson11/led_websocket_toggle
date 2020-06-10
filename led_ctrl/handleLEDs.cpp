/* controls the leds base on messages received from the job queue */

/**
 * Requests are json objects of the form
 * 
 * request = {
 *   type: [reqType],
 *   args: {
 *     arg1: val1
 *     ...
 *   };
 * };
 * 
 * where [reqType] is the type of request and args is a object containing
 * name-value pairs of the arguments.
 * 
 * valid requests:
 *   getState - respond with the value of all state variables
 *   modify - modify state variables and respond with new values
 *
 * 
 * Responses are json objects of the form
 * 
 * response = {
 *   state: {
 *     var1: val1,
 *     var2: val2,
 *     ...
 *   }
 * };
 * 
 * where (var1, val1)... are name-value pairs of state variables which
 * are to be sent back to the client.
 * 
 * Some commands may include an id property, in which case the same property-value
 * pair should be added to the response.
 */

#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include <string>
#include <iostream>

#include "handleLEDs.h"
#include "LEDDriver.h"
#include "signalHandler.h"

#include "json.hpp"

using nlohmann::json;

// valid requests
#define REQUEST_STATE       "getState"
#define REQUEST_MODE        "setMode"
#define REQUEST_PATTERN     "setPattern"
#define REQUEST_PERIODMS    "setPeriod"

void* handleLEDs(void *_arg)
{
    HandleLEDsArg *arg = (HandleLEDsArg *)_arg;

    LEDDriver leds(arg->physLEDNumbers, arg->nLEDs, "led_ctrl");
    enum LEDDriverMode mode = LED_MODE_TOGGLE;
    leds.setMode("toggle");

    timespec ts = {0};
    TimespecArithmetic nextTickTime(ts);
    TimespecArithmetic waitTime(ts);
    TimespecArithmetic currentTime(ts);
    TimespecArithmetic zeroTime(ts);

    // setup poll structures
    pollfd fds[2];
    fds[0].fd = arg->jobQueueFD;
    fds[0].events = POLLIN;
    fds[1].fd = arg->shutdownFD;
    fds[1].events = POLLIN;

    bool ShutdownProgram = false;
    while (!ShutdownProgram) {
        clock_gettime(CLOCK_MONOTONIC_COARSE, currentTime.data());
        waitTime = nextTickTime - currentTime;

        if (waitTime.getTimespec().tv_sec < 0) {
            waitTime = zeroTime;
        }

        if (mode == LED_MODE_TOGGLE) {
            // wait for data on job queue or shutdown event
            ppoll(fds, 2, nullptr, nullptr);
            if (fds[1].revents & POLLIN) {
                ShutdownProgram = true;
                continue;
            }

            char *buf;
            int bytesRead = read(arg->jobQueueFD, &buf, sizeof(buf));

            if (bytesRead > 0) {
                // parse request
                json request = json::parse(buf);

                // handle request
                // TODO: put json work in try...catch block
                std::cout << "handleLED: " << request << std::endl;
                std::string reqType = request["type"];
                json response;
                if (request.contains("id")) {
                    response["id"] = request["id"];
                }

                if (reqType == REQUEST_STATE) {
                    response["state"] = leds.getState();
                } else if (reqType == REQUEST_MODE) {
                    response["state"] = leds.setMode(request["arg"]["mode"]);
                } else if (reqType == REQUEST_PATTERN) {
                    response["state"] = leds.setPattern(request["arg"]["pattern"]);
                } else if (reqType == REQUEST_PERIODMS) {
                    response["state"] = leds.setPeriod(request["arg"]["period"]);
                } else {
                    // invalid request type
                }

                // send response
                std::cout << "Here\n";
                std::string jsonResponse = response.dump();
                char *buf = new char[1024];  // TODO: use memory pool
                strcpy(buf, jsonResponse.c_str());
                write(arg->msgQueueFD, &buf, sizeof(buf));
            }
        } else if (mode == LED_MODE_PATTERN) {
            // wait for data on job queue or timeout

            // if not timeout
            //   read request form job queue
            //   parse and check requested operation
            //   create response
            //   send response
            // else
            //   nextTick = tick();
        } else {
            // invalid mode
            mode = LED_MODE_TOGGLE;
        }
    }

    return 0;
}