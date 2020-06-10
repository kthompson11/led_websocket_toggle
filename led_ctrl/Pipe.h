/* class that takes care of creation and cleanup of pipes */

#ifndef PIPE_H
#define PIPE_H

class Pipe
{
public:
    Pipe();
    ~Pipe();
    int getWriteFD();
    int getReadFD();
private:
    int fds[2];
};

#endif // PIPE_H