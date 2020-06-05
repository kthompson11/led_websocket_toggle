#include <iostream>
#include <sstream>
#include <iomanip>

#include "TimespecArithmetic.h"

static const long MaxNSec = 1000000000;

TimespecArithmetic::TimespecArithmetic()
{
    time = new timespec;
}

TimespecArithmetic::TimespecArithmetic(const TimespecArithmetic &other): TimespecArithmetic()
{
    time->tv_sec = other.time->tv_sec;
    time->tv_nsec = other.time->tv_nsec;
}

TimespecArithmetic::TimespecArithmetic(TimespecArithmetic &&other)
{
    time = other.time;
    other.time = nullptr;
}

TimespecArithmetic::TimespecArithmetic(const timespec &time): TimespecArithmetic()
{
    TimespecArithmetic();

    this->time->tv_sec = time.tv_sec;
    this->time->tv_nsec = time.tv_nsec;
}

TimespecArithmetic::TimespecArithmetic(long ms): TimespecArithmetic()
{
    TimespecArithmetic();

    time->tv_sec = ms / 1000;
    time->tv_nsec = (ms % 1000) * (MaxNSec / 1000);
}

TimespecArithmetic::~TimespecArithmetic()
{
    if (time != nullptr) {
        delete time;
    }
}

TimespecArithmetic TimespecArithmetic::operator+(const TimespecArithmetic &other) const
{
    timespec lhs = *time;
    timespec rhs = other.getTimespec();
    timespec res;

    res.tv_sec = lhs.tv_sec + rhs.tv_sec;
    res.tv_nsec = lhs.tv_nsec + rhs.tv_nsec;
    if ((res.tv_nsec >= MaxNSec) || (res.tv_nsec < 0)) {
        res.tv_nsec = res.tv_nsec - MaxNSec;
        res.tv_sec = res.tv_sec + 1;
    }

    TimespecArithmetic result(res);
    return result;
}

TimespecArithmetic TimespecArithmetic::operator-(const TimespecArithmetic &other) const
{
    timespec lhs = *time;
    timespec rhs = other.getTimespec();
    timespec res;

    res.tv_sec = lhs.tv_sec - rhs.tv_sec;
    res.tv_nsec = lhs.tv_nsec - rhs.tv_nsec;
    if ((res.tv_nsec > lhs.tv_nsec) || (res.tv_nsec < 0)) {
        res.tv_nsec = res.tv_nsec + MaxNSec;
        res.tv_sec = res.tv_sec - 1;
    }

    return TimespecArithmetic(res);
}

TimespecArithmetic& TimespecArithmetic::operator=(const TimespecArithmetic &rhs)
{
    if (this != &rhs) {
        time->tv_sec = rhs.time->tv_sec;
        time->tv_nsec = rhs.time->tv_nsec;
    }

    return *this;
}

TimespecArithmetic& TimespecArithmetic::operator=(TimespecArithmetic &&rhs)
{
    if (this != &rhs) {
        std::swap(time, rhs.time);
    }

    return *this;
}

std::string TimespecArithmetic::toString()
{
    std::string res;
    std::ostringstream ostream;

    ostream << time->tv_sec << "." << std::setfill('0') << std::right << std::setw(9) << time->tv_nsec;

    return ostream.str();
}