#include <sys/eventfd.h>
#include <unistd.h>
#include <signal.h>

#include <system_error>

#include "SigintHandler.h"

/******************** Static Members ********************/

int SigintHandler::shutdownFD;
bool SigintHandler::isConstructed = false;
void SigintHandler::handler(int signum) {
    uint64_t val = 1;
    write(shutdownFD, &val, sizeof(val));
}

/******************** Constructors ********************/

SigintHandler::SigintHandler()
{
    // create shutdown file descriptor
    shutdownFD = eventfd(0, 0);
    if (shutdownFD == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    // register handler
    sigset_t sigmask = {0};
    sigprocmask(SIG_BLOCK, &sigmask, NULL);
    sigaddset(&sigmask, SIGINT);
    struct sigaction act;
    act.sa_handler = handler;
    int status = sigaction(SIGINT, &act, NULL);
    if (status == -1) {
        close(shutdownFD);
        throw std::system_error(errno, std::generic_category());
    }

    isConstructed = true;
}

SigintHandler::~SigintHandler()
{
    isConstructed = false;
    signal(SIGINT, SIG_DFL);
    close(shutdownFD);
}

/******************** Member Functions ********************/

int SigintHandler::getShutdownFD()
{
    return shutdownFD;
}