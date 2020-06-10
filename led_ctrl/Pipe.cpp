#include <unistd.h>

#include <system_error>

#include "Pipe.h"

Pipe::Pipe()
{
    if (pipe(fds) == -1) {
        throw std::system_error(errno, std::generic_category());
    }
}

Pipe::~Pipe()
{
    close(fds[0]);
    close(fds[1]);
}

int Pipe::getReadFD()
{
    return fds[0];
}

int Pipe::getWriteFD()
{
    return fds[1];
}