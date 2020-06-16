/* Minimal wrapper around needed gpiod */

#ifndef MANAGEDGPIOD_H
#define MANAGEDGPIOD_H

#include <string>
#include <vector>

#include <gpiod.h>

namespace GPIOD {

class Chip
{
public:
    Chip(const std::string &path);
    ~Chip();

    // deleted methods
    Chip(const Chip&) = delete;
    Chip& operator=(const Chip&) = delete;
    Chip(const Chip&&) = delete;
    Chip& operator=(const Chip&&) = delete;
protected:
    gpiod_chip* getChip();
private:
    gpiod_chip *chip = nullptr;
friend class LineBulk;
};

class LineBulk
{
public:
    LineBulk(Chip &chip, std::vector<unsigned int> &offsets);
    ~LineBulk();
    void requestOutput(const std::string &consumerName, const std::vector<int> &values);
    std::vector<int> getValues();
    void setValues(const std::vector<int> &values);
    unsigned int getNLines() { return lines.num_lines; }

    // deleted methods
    LineBulk(const LineBulk&) = delete;
    LineBulk& operator=(const LineBulk&) = delete;
    LineBulk(const LineBulk&&) = delete;
    LineBulk& operator=(const LineBulk&&) = delete;
private:
    gpiod_line_bulk lines;
    bool isOutputRequested = false;
};

}  // GPIOD

#endif // MANAGEDGPIOD_H