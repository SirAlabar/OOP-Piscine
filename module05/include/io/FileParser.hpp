#ifndef FILEPARSER_HPP
#define FILEPARSER_HPP

#include <string>
#include <vector>
#include <fstream>

// Base class for all file parsers - handles common file operations
class FileParser
{
protected:
	std::string _filepath;
	std::ifstream _file;

	FileParser(const std::string& filepath);
	virtual ~FileParser() = default;

	// Read all lines from file
	std::vector<std::string> readLines();

	// Check if line is empty or comment
	bool isEmptyOrComment(const std::string& line) const;

	// Trim whitespace from string
	std::string trim(const std::string& str) const;

public:
	// Static method to check if file exists
	static bool fileExists(const std::string& filepath);

	// Static method to validate file can be read
	static void validateFile(const std::string& filepath);
};

#endif