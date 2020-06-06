#ifndef SOCKET_H
#define SOCKET_H

#include <sys/time.h>

class Socket
{
public:
    ssize_t read(void *buf, size_t count);
    ssize_t readWithTimeout(void *buf, size_t count, timespec *timeoutMS);
    ssize_t write(const void *buf, size_t count);
    ssize_t writeWithTimeout(void *buf, size_t count, timespec *timeoutMS);
    void close();
    virtual ~Socket();
protected:
    Socket();
    Socket(int fd);
    int sockfd = 0;
    int isConnected = false;
};

#endif // SOCKET_H