#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
public:
    ssize_t read(void *buf, size_t count);
    ssize_t readWithTimeout(void *buf, size_t count, unsigned int timeoutMS);
    ssize_t write(const void *buf, size_t count);
    ssize_t writeWithTimeout(void *buf, size_t count, unsigned int timeoutMS);
    void close();
    virtual ~Socket();
protected:
    Socket();
    Socket(int fd);
    int sockfd = 0;
    int isConnected = false;
};

#endif // SOCKET_H