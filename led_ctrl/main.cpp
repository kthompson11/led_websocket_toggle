
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <gpiod.h>

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

    int status, err;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        err = errno;
        std::cerr << "socket() error (errno = " << err << ")\n";
        return -1;
    }

    struct sockaddr_un sockaddr = {0};
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, SOCKET_PATH);
    status = bind(sockfd, (struct sockaddr *)&sockaddr, SUN_LEN(&sockaddr));
    if (status == -1) {
        err = errno;
        std::cerr << "bind() error (errno = " << err << ")\n";
        close(sockfd);
        return -1;
    }

    status = listen(sockfd, 5);
    if (status == -1) {
        err = errno;
        std::cerr << "listen() error (errno = " << err << ")\n";
        close(sockfd);
        return -1;
    }

    std::cout << "Before accept\n";
    int client_fd = accept(sockfd, NULL, NULL);

    while (!AbortProgram) {
        char buf[1024] = {0};
        std::cout << "Before read\n";
        int bytesRead = read(client_fd, buf, 100);
        std::cout << "After read\n";
        if (bytesRead > 0) {
            buf[bytesRead] = '\0';
            std::cout << buf << std::endl;
        }
    }

    close(client_fd);
    close(sockfd);
    unlink("/tmp/led_ctrl");

    return 0;


    struct gpiod_chip *chip;
	chip = gpiod_chip_open("/dev/gpiochip0");
	if (!chip)
		return -1;

	struct gpiod_line *line;
	line = gpiod_chip_get_line(chip, 7);
	if (!line) {
		gpiod_chip_close(chip);
		return -1;
	}

	int req = gpiod_line_request_output(line, "gpio_test", 0);
	if (req) {
		gpiod_chip_close(chip);
		return -1;
	}

    gpiod_line_set_value(line, 1);

    while (true) {}

    return 0;
}