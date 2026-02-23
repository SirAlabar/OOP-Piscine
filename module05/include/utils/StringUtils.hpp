#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <sstream>

class StringUtils
{
public:

    // Build the opening JSON field for a command record: {"t":<timestamp>
    // Caller appends remaining fields and closing brace.
    static std::string serializeHeader(double timestamp)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << "{\"t\":" << timestamp;
        return ss.str();
    }

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

    // Parse a double from a token; throws if the token contains non-numeric characters.
    static double parseDouble(const std::string& token, const std::string& fieldName)
    {
        std::size_t pos = 0;
        double value = std::stod(token, &pos);

        if (pos != token.size())
        {
            throw std::runtime_error("Invalid numeric value for '" + fieldName + "': '" + token + "'");
        }

        return value;
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