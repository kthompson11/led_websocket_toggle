#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/select.h>

#include <system_error>
#include <iostream>

#include "Socket.h"

Socket::Socket()
{
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::system_error(errno, std::generic_category());
    }
}

Socket::Socket(int fd)
{
    sockfd = fd;
    isConnected = true;
}

Socket::~Socket()
{
    close();
}

ssize_t Socket::read(void *buf, size_t count)
{
    if (!isConnected) {
        return -ENOTCONN;
    }

    return ::read(sockfd, buf, count);
}

/** 
 * Attempts to read from the socket and aborts if no data on socket before
 * specified timeout. On successful read, updates timeoutMS with the wait time
 * remaining.
 * 
 * return: number of bytes read
 */
ssize_t Socket::readWithTimeout(void *buf, size_t count, timespec *timeoutMS)
{
    if (!isConnected) {
        return -ENOTCONN;
    }

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    timeval timeout;
    TIMESPEC_TO_TIMEVAL(&timeout, timeoutMS);

    int status = select(sockfd + 1, &rfds, NULL, NULL, &timeout);
    if (status == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    TIMEVAL_TO_TIMESPEC(&timeout, timeoutMS);
    if (status > 0) {
        // read data from socket
        return read(buf, count);
    }

    return 0;
}

ssize_t Socket::write(const void *buf, size_t count)
{
    if (!isConnected) {
        return -ENOTCONN;
    }

    return ::write(sockfd, buf, count);
}

void Socket::close()
{
    if (sockfd != 0) {
        ::close(sockfd);
        sockfd = 0;
    }
}