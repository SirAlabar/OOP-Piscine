#ifndef FILEPARSER_HPP
#define FILEPARSER_HPP

#include <string>
#include <vector>
#include <fstream>

// Base class for all file parsers.
// Handles common file operations and provides shared helpers so that derived
// parsers never duplicate error formatting, line counting, or whitespace logic.
class FileParser
{
protected:
    std::string   _filepath;
    std::ifstream _file;
    int           _lineNumber;  // Tracks current line; reset to 0 in constructor.

    FileParser(const std::string& filepath);
    virtual ~FileParser() = default;

    // Read all non-empty, non-comment lines from the file.
    // Strips inline '#' comments and trims whitespace.
    std::vector<std::string> readLines();

    // Returns true when a line should be skipped (empty or starts with '#').
    bool isEmptyOrComment(const std::string& line) const;

    // Trim leading and trailing whitespace.
    std::string trim(const std::string& str) const;

    // Throw a std::runtime_error formatted as:
    //   "Error at line N: <message>\nContent: <lineContent>"
    // Always uses the current value of _lineNumber.
    // Marked [[noreturn]] so the compiler knows it never returns.
    [[noreturn]] void throwLineError(
        const std::string& message,
        const std::string& lineContent) const;

public:
    // Returns true when the file exists and is a regular file.
    static bool fileExists(const std::string& filepath);

    // Throws std::runtime_error when the file cannot be opened for reading.
    static void validateFile(const std::string& filepath);
};

#endif