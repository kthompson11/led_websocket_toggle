#include <stdexcept>
#include <system_error>
#include <iostream>

#include "ManagedGpiod.h"

using namespace GPIOD;

/************************ Chip Methods ************************/

Chip::Chip(const std::string &path)
{
    chip = gpiod_chip_open(path.c_str());
    if (chip == nullptr) {
        std::cerr << "Error calling Chip(" + path + ")\n";
        throw std::system_error(errno, std::generic_category());
    }
}

Chip::~Chip()
{
    gpiod_chip_close(chip);
}

gpiod_chip* Chip::getChip()
{
    return chip;
}

/************************ LineBulk Methods ************************/

LineBulk::LineBulk(Chip &chip, std::vector<unsigned int> &offsets)
{
    int status = gpiod_chip_get_lines(chip.getChip(), offsets.data(), offsets.size(), &lines);
    if (status == -1) {
        std::cerr << "Error calling gpiod_chip_get_lines()\n";
        throw std::system_error(errno, std::generic_category());
    }
}

LineBulk::~LineBulk()
{
    gpiod_line_release_bulk(&lines);
}

void LineBulk::requestOutput(const std::string &consumerName, const std::vector<int> &values)
{
    if (consumerName.size() == 0) {
        std::string msg = "Invalid consumerName (length == 0)";
        throw std::invalid_argument(msg);
    }

    if (values.size() != lines.num_lines) {
        std::string msg = "Invalid values (values.size() != number of lines)";
        throw std::invalid_argument(msg);
    }

    int status = gpiod_line_request_bulk_output(&lines, consumerName.c_str(), values.data());
    if (status == -1) {
        std::cerr << "Error calling gpiod_line_request_bulk_output()\n";
        throw std::system_error(errno, std::generic_category());
    }
}

std::vector<int> LineBulk::getValues()
{
    std::vector<int> values(lines.num_lines, 0);
    int status = gpiod_line_get_value_bulk(&lines, values.data());
    if (status == -1) {
        std::cerr << "Error calling gpiod_line_get_value_bulk()\n";
        throw std::system_error(errno, std::generic_category());
    }

    return values;
}

void LineBulk::setValues(const std::vector<int> &values)
{
    if (values.size() != lines.num_lines) {
        std::string msg = "Invalid values (values.size() != number of lines)";
        throw std::invalid_argument(msg);
    }

    int status = gpiod_line_set_value_bulk(&lines, values.data());
    if (status == -1) {
        std::cerr << "Error calling gpiod_line_set_value_bulk()\n";
        throw std::system_error(errno, std::generic_category());
    }
}