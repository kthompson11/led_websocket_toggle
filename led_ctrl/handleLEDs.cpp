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

#include "json.hpp"

using nlohmann::json;

// valid requests
#define REQUEST_STATE       "getState"
#define REQUEST_MODE        "setMode"
#define REQUEST_PATTERN     "setPattern"
#define REQUEST_PERIODMS    "setPeriod"
#define REQUEST_TOGGLE      "toggle"

void* handleLEDs(void *_arg)
{
    HandleLEDsArg *arg = (HandleLEDsArg *)_arg;

    LEDDriver leds(arg->physLEDNumbers, "led_ctrl");

    // Start LEDDriver thread
    pthread_t ledDriverThread;
    LEDDriverArg ledDriverArg = {
        .shutdownFD = arg->shutdownFD,
        .leds = leds
    };
    if (pthread_create(&ledDriverThread, nullptr, LEDDriver::threadStart, &ledDriverArg)) {
        std::cerr << "Could not create LEDDriver thread\n";
        throw std::system_error(errno, std::generic_category());
    }

    // setup poll structures
    pollfd fds[2];
    fds[0].fd = arg->jobQueueFD;
    fds[0].events = POLLIN;
    fds[1].fd = arg->shutdownFD;
    fds[1].events = POLLIN;

    bool ShutdownProgram = false;
    while (!ShutdownProgram) {
        // wait for data on job queue or shutdown event
        int status = ppoll(fds, 2, nullptr, nullptr);
        if (status == -1) {
                std::cerr << "ppoll() error\n";
                throw std::system_error(errno, std::generic_category());
        }

        // check if shutdown event occurred
        if (fds[1].revents & POLLIN) {
            ShutdownProgram = true;
            continue;
        }

        // process incoming request
        char *buf;
        int bytesRead = read(arg->jobQueueFD, &buf, sizeof(buf));

        if (bytesRead > 0) {
            // parse request
            json request = json::parse(buf);
            delete[] buf;

            std::cout << "request: " << request << std::endl;
            std::string reqType = request["type"];
            json response;
            if (request.contains("id")) {
                response["id"] = request["id"];
            }

            // handle request
            if (reqType == REQUEST_STATE) {
                response["state"] = leds.getState();
            } else if (reqType == REQUEST_MODE) {
                std::cout << "setting mode\n";
                response["state"] = leds.setMode(request["arg"]["mode"]);
                std::cout << "finished setting mode\n";
            } else if (reqType == REQUEST_PATTERN) {
                response["state"] = leds.setPattern(request["arg"]["pattern"]);
            } else if (reqType == REQUEST_PERIODMS) {
                response["state"] = leds.setPeriod(request["arg"]["period"]);
            } else if (reqType == REQUEST_TOGGLE) {
                response["state"] = leds.toggleLed(request["arg"]["iled"]);
            } else {
                // invalid request type
                std::cerr << "Invalid request\n";
            }

            // send response
            std::cout << "response: " << response << std::endl << std::endl;
            std::string jsonResponse = response.dump();
            char *buf = new char[1024];  // TODO: use memory pool
            strcpy(buf, jsonResponse.c_str());
            write(arg->msgQueueFD, &buf, sizeof(buf));
        }
    }

    // wait for led driver thread to return
    pthread_join(ledDriverThread, nullptr);

    return 0;
}