# LED WebSocket Toggle

This project is a test of using websockets to control the state of hardware (e.g. LEDs) from a web inteface. Hardware state is automatically forwarded to all connected clients so that all views of the hardware are consistent.

The system has two modes of operation: toggle and pattern. In togle mode, LEDs can be toggled on or off by clicking on them. In pattern mode, LEDs can display one of a number of set patterns at a settable tick period.

The software runs as is on a Raspberry Pi 1B. It may also work with newer devices if the pin assignments are updated.

## Requirements

- Raspberry Pi 1B with Raspbian
- nginx (or other web server capable of acting as a reverse proxy)
- Node.js

## Components

- led_ctrl - Controls the LEDs
- node_server - Receives requests from clients and forwards them to `led_ctrl` using UNIX sockets. Also updates all connected clients regarding changes in system state.
- webui - Angular web interface for the system

## Setup

### Hardware

See `example_schematic.pdf` for a schematic of the default setup. LEDs should be arranged in a line in the same order as declared for the `physLedNumbers` variable in `led_ctrl/main.cpp`.

### Software

1. Install nginx on the Raspberry Pi and set up to proxy WebSocket requests as shown in `nginx_example.conf`
2. Install Angular using npm by using `npm install -g @angular/cli`
3. Navigate to `webui/` build the webui by using `ng build --prod`
4. Copy the contents of `dist/webui` to the Raspberry Pi web server root directory (typically `/var/www/html`)
5. Copy `led_ctrl/` and `node_server/` to Raspberry Pi
6. On the Raspberry Pi:
    1. Navigate to `led_ctrl/` and build it by using `make`
    2. Run the `led_ctrl` executable
    3. Using another terminal, navigate to the `node_server/` directory and run `node node_server.js`
7. Open a web page to the Raspberry Pi IP address to begin controlling the GPIOs/LEDs.

## Customizaton

The following and be customized from `led_ctrl/main.cpp` file:

- number of LEDs and the corresponding BCM2835 pins (physLedNumbers)