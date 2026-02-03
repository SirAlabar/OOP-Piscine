#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class StringUtils
{
public:
    // Split a string by whitespace into tokens
    static std::vector<std::string> splitTokens(const std::string& line)
    {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (iss >> token)
        {
            tokens.push_back(token);
        }

        return tokens;
    }
};

#endif
