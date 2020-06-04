#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <system_error>

#include "UnixServerSocket.h"

UnixServerSocket::UnixServerSocket(const char *path)
{
    // bind
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, path);
    if (bind(sockfd, (struct sockaddr *)&sockaddr, SUN_LEN(&sockaddr)) == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    // listen
    if (listen(sockfd, 5) == -1) {
        throw std::system_error(errno, std::generic_category());
    }
}

UnixServerSocket::~UnixServerSocket()
{

}

Socket* UnixServerSocket::accept()
{
    // don't care about peer socket address
    int fd = ::accept(sockfd, NULL, NULL);
    return new UnixServerSocket(fd);
}