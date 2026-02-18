#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>

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

    // Trim leading and trailing whitespace
    static std::string trim(const std::string& s)
    {
        std::size_t start = s.find_first_not_of(" \t\r\n");

        if (start == std::string::npos)
        {
            return "";
        }

        std::size_t end = s.find_last_not_of(" \t\r\n");

        return s.substr(start, end - start + 1);
    }

    // Escape a string for JSON output (handles ", \, \n)
    static std::string escapeJson(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());

        for (char c : s)
        {
            if (c == '"')
            {
                out += "\\\"";
            }
            else if (c == '\\')
            {
                out += "\\\\";
            }
            else if (c == '\n')
            {
                out += "\\n";
            }
            else
            {
                out += c;
            }
        }

        return out;
    }

    // Unescape a JSON string value (handles \", \\, \n)
    static std::string unescapeJson(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());

        for (std::size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size())
            {
                ++i;

                if (s[i] == '"')
                {
                    out += '"';
                }
                else if (s[i] == '\\')
                {
                    out += '\\';
                }
                else if (s[i] == 'n')
                {
                    out += '\n';
                }
                else
                {
                    out += s[i];
                }
            }
            else
            {
                out += s[i];
            }
        }

        return out;
    }
};

#endif
