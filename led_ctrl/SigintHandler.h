#ifndef SIGINTHANDLER_H
#define SIGINTHANDLER_H

class SigintHandler
{
public:
    SigintHandler();
    ~SigintHandler();
    int getShutdownFD();
private:
    static int shutdownFD;
    static bool isConstructed;
    static void handler(int signum);
};

#endif // SIGINTHANDLER_H