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

    Graph* parse();

private:
    void parseLine(const std::string& line, Graph* graph);
};

#endif