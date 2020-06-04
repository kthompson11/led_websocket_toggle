#ifndef UNIXSERVERSOCKET_H
#define UNIXSERVERSOCKET_H

#include "Socket.h"

class UnixServerSocket: public Socket
{
public:
    UnixServerSocket(const char *path);
    ~UnixServerSocket();
    Socket* accept();
private:
    UnixServerSocket(int fd): Socket(fd) {}
    struct sockaddr_un sockaddr = {0};
};

#endif // UNIXSERVERSOCKET_H