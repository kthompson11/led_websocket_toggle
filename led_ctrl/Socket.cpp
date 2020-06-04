#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <system_error>

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