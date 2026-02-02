#include "io/FileParser.hpp"
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <algorithm>

FileParser::FileParser(const std::string& filepath) : _filepath(filepath)
{
	_file.open(filepath);
	if (!_file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filepath);
	}
}

std::vector<std::string> FileParser::readLines()
{
	std::vector<std::string> lines;
	std::string line;

	while (std::getline(_file, line))
	{
		line = trim(line);
		if (!isEmptyOrComment(line))
		{
			lines.push_back(line);
		}
	}

	return lines;
}

bool FileParser::isEmptyOrComment(const std::string& line) const
{
	return line.empty() || line[0] == '#';
}

std::string FileParser::trim(const std::string& str) const
{
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
	{
		return "";
	}

	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

bool FileParser::fileExists(const std::string& filepath)
{
	return std::filesystem::exists(filepath) 
	    && std::filesystem::is_regular_file(filepath);
}

void FileParser::validateFile(const std::string& filepath)
{
	if (!std::filesystem::exists(filepath))
	{
		throw std::runtime_error("File does not exist: " + filepath);
	}

	if (!std::filesystem::is_regular_file(filepath))
	{
		throw std::runtime_error("Path is not a regular file: " + filepath);
	}

	// Check read permissions by trying to open
	std::ifstream test(filepath);
	if (!test.good())
	{
		throw std::runtime_error("File is not readable: " + filepath);
	}
}