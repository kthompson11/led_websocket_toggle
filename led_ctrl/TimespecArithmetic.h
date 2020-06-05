#ifndef TIMESPECARITHMETIC_H
#define TIMESPECARITHMETIC_H

#include <sys/time.h>

#include <string>

class TimespecArithmetic
{
public:
    TimespecArithmetic();
    TimespecArithmetic(const TimespecArithmetic &other);
    TimespecArithmetic(TimespecArithmetic &&other);
    TimespecArithmetic(const timespec &time);
    TimespecArithmetic(long ms);
    ~TimespecArithmetic();

    timespec *data() { return time; }  // exposes pointer to internal timespec
    timespec getTimespec() const { return *time; }  // returns a copy of internal timespec
    TimespecArithmetic operator+(const TimespecArithmetic &rhs) const;
    TimespecArithmetic operator-(const TimespecArithmetic &rhs) const; // rhs must be later or same time as this
    TimespecArithmetic& operator=(const TimespecArithmetic &other);
    TimespecArithmetic& operator=(TimespecArithmetic &&other);
    std::string toString();
private:
    timespec *time = nullptr;
};

#endif // TIMESPECARITHMETIC_H