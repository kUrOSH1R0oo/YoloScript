// utils.cpp — YoloScript file/stdin utilities
#include "utils.hpp"
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::string readFromStdin() {
    std::ostringstream oss;
    oss << std::cin.rdbuf();
    return oss.str();
}

std::string readFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    std::ostringstream oss;
    oss << file.rdbuf();
    if (file.bad())
        throw std::runtime_error("Error reading file: " + filename);

    return oss.str();
}
