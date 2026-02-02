#ifndef TRAINCONFIGPARSER_HPP
#define TRAINCONFIGPARSER_HPP

#include "io/FileParser.hpp"
#include "patterns/factories/TrainFactory.hpp"
#include <vector>

// Parses train configuration file
class TrainConfigParser : public FileParser
{
public:
	TrainConfigParser(const std::string& filepath);
	~TrainConfigParser() = default;

	// Parse file and return train configurations
	std::vector<TrainConfig> parse();

private:
	TrainConfig parseLine(const std::string& line);
};

#endif