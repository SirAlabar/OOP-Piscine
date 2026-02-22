#include "io/RailNetworkParser.hpp"
#include "utils/StringUtils.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <stdexcept>

RailNetworkParser::RailNetworkParser(const std::string& filepath)
    : FileParser(filepath)
{
}

Graph* RailNetworkParser::parse()
{
    Graph* graph = new Graph();

    try
    {
        std::vector<std::string> lines = readLines();

        for (const auto& line : lines)
        {
            _lineNumber++;
            try
            {
                parseLine(line, graph);
            }
            catch (const std::exception& e)
            {
                throwLineError(e.what(), line);
            }
        }

        if (!graph->isValid())
        {
            throw std::runtime_error("Graph validation failed after parsing");
        }
    }
    catch (const std::exception&)
    {
        delete graph;
        throw;
    }

    return graph;
}

void RailNetworkParser::parseLine(const std::string& line, Graph* graph)
{
    auto tokens = StringUtils::splitTokens(line);

    if (tokens.empty())
    {
        return;
    }

    const std::string& keyword = tokens[0];

    if (keyword == "Node")
    {
        if (tokens.size() != 2)
        {
            throw std::runtime_error("Invalid Node format. Expected: Node <n>");
        }

        const std::string& nodeName = tokens[1];

        if (nodeName.empty())
        {
            throw std::runtime_error("Node name cannot be empty");
        }

        if (graph->hasNode(nodeName))
        {
            throw std::runtime_error("Duplicate node: '" + nodeName + "'");
        }

        NodeType type = (nodeName.rfind("RailNode", 0) == 0)
                      ? NodeType::JUNCTION
                      : NodeType::CITY;

        graph->addNode(new Node(nodeName, type));
        return;
    }

    if (keyword == "Rail")
    {
        if (tokens.size() != 5)
        {
            throw std::runtime_error(
                "Invalid Rail format. Expected: Rail <nodeA> <nodeB> <length> <speedLimit>");
        }

        const std::string& nodeA = tokens[1];
        const std::string& nodeB = tokens[2];

        if (nodeA == nodeB)
        {
            throw std::runtime_error(
                "Rail cannot connect node to itself: '" + nodeA + "'");
        }

        if (!graph->hasNode(nodeA))
        {
            throw std::runtime_error("Unknown node: '" + nodeA + "'");
        }

        if (!graph->hasNode(nodeB))
        {
            throw std::runtime_error("Unknown node: '" + nodeB + "'");
        }

        double length;
        double speed;

        try
        {
            length = StringUtils::parseDouble(tokens[3], "length");
            speed  = StringUtils::parseDouble(tokens[4], "speedLimit");
        }
        catch (...)
        {
            throw std::runtime_error("Length and speed must be numeric values");
        }

        if (length <= 0.0)
        {
            throw std::runtime_error("Rail length must be positive");
        }

        if (speed <= 0.0)
        {
            throw std::runtime_error("Speed limit must be positive");
        }

        graph->addRail(
            new Rail(graph->getNode(nodeA), graph->getNode(nodeB), length, speed));
        return;
    }

    throw std::runtime_error("Unknown keyword: '" + keyword + "'");
}