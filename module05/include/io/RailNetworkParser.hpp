#ifndef RAILNETWORKPARSER_HPP
#define RAILNETWORKPARSER_HPP

#include "io/FileParser.hpp"
#include "core/Graph.hpp"

// Parses railway network file and builds Graph
class RailNetworkParser : public FileParser
{
public:
	RailNetworkParser(const std::string& filepath);
	~RailNetworkParser() = default;

	// Parse file and return graph
	Graph* parse();

private:
    std::vector<std::string> splitTokens(const std::string& line);
	void parseLine(const std::string& line, Graph* graph);
	void parseNode(const std::string& line, Graph* graph);
	void parseRail(const std::string& line, Graph* graph);
};

#endif