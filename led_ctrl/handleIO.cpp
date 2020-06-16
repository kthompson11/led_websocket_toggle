/* handles socket input/output and puts messages into the job queue */
// TODO: use a memory pool for the buffers

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#include <system_error>
#include <cstring>
#include <iostream>

#include "handleIO.h"

static const int BufSize = 1024;

void* handleIO(void *_arg)
{
    HandleIOArg *arg = (HandleIOArg *)_arg;

    // create passive socket to listen for connections
    int listenerFD = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un sockaddr = {0};
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, arg->socketPath.c_str());
    unlink(arg->socketPath.c_str());
    if (bind(listenerFD, (struct sockaddr *)&sockaddr, SUN_LEN(&sockaddr)) == -1) {
        std::cerr << "bind() error\n";
        throw std::system_error(errno, std::generic_category());
    }

    if (listen(listenerFD, 5) == -1) {
        std::cerr << "listen() error\n";
        throw std::system_error(errno, std::generic_category());
    }

    // setup poll structures
    pollfd fds[4];
    // setup socket output fd for reads
    fds[0].fd = arg->msgQueueFD;
    fds[0].events = POLLIN;
    // setup listen socket for reads
    fds[1].fd = listenerFD;
    fds[1].events = POLLIN;
    fds[2].fd = arg->shutdownFD;
    fds[2].events = POLLIN;
    nfds_t nfds = 3;
    bool isConnected = false;
    int socketFD;

    bool shutdownProgram = false;
    while (!shutdownProgram) {
        if (poll(fds, nfds, -1) == -1)
        {
            std::cerr << "poll() error\n";
            throw std::system_error(errno, std::generic_category());
        }

        // check for the pending read
        for (nfds_t i = 0; i < nfds; ++i) {
            pollfd fdStruct = fds[i];
            if (fdStruct.revents & POLLIN) {
                // read pending for this fd
                int readFD = fdStruct.fd;
                
                if (isConnected) {
                    if (readFD == listenerFD) {
                        // accept and close connection (only one connection allowed)
                        int fd = accept(readFD, nullptr, nullptr);
                        close(fd);
                        std::cout << "Closed duplicate socket\n";
                    } else if (readFD == arg->msgQueueFD) {
                        // copy data from message queue to socket
                        char *buf;
                        read(readFD, &buf, sizeof(buf));
                        write(socketFD, buf, strlen(buf));
                        delete[] buf;
                    } else if (readFD == arg->shutdownFD) {
                        shutdownProgram = true;
                    } else if (readFD == socketFD) {
                        // copy from socket to job queue
                        char *buf = new char[BufSize];
                        ssize_t bytesRead = read(socketFD, buf, BufSize - 1);
                        buf[bytesRead] = '\0';
                        if (bytesRead == 0) {
                            // socket is closed
                            delete[] buf;
                            close(socketFD);
                            isConnected = false;
                            nfds = nfds - 1;
                            std::cout << "Socket closed\n";
                        } else if (bytesRead == -1) {
                            // error
                            delete[] buf;
                        } else {
                            // put buffer on job queue
                            write(arg->jobQueueFD, &buf, sizeof(buf));
                        }
                    }
                } else { // not connected
                    if (readFD == listenerFD) {
                        // accept the connection
                        int fd = accept(readFD, nullptr, nullptr);
                        socketFD = fd;
                        fds[nfds].fd = fd;
                        fds[nfds].events = POLLIN;
                        nfds = nfds + 1;
                        isConnected = true;
                        std::cout << "Socket connected\n";
                    } else if (readFD == arg->shutdownFD) {
                        shutdownProgram = true;
                    } else if (readFD == arg->msgQueueFD) {
                        // discard data
                        char *buf;
                        read(readFD, &buf, sizeof(buf));
                        delete[] buf;
                    }
                }
            }
        }
    }

    // cleanup
    if (isConnected) {
        close(socketFD);
    }
    close(listenerFD);

    return 0;
}