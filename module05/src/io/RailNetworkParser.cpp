#include "io/RailNetworkParser.hpp"
#include "core/Node.hpp"
#include "core/Rail.hpp"
#include <sstream>
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
		int lineNumber = 0;

		for (const auto& line : lines)
		{
			lineNumber++;
			try
			{
				parseLine(line, graph);
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(
					"Error at line " + std::to_string(lineNumber) + 
					": " + e.what() + "\nContent: " + line
				);
			}
		}

		if (!graph->isValid())
		{
			throw std::runtime_error("Graph validation failed after parsing");
		}
	}
	catch (const std::exception& e)
	{
		delete graph;
		throw;
	}

	return graph;
}

void RailNetworkParser::parseLine(const std::string& line, Graph* graph)
{
	std::istringstream iss(line);
	std::string keyword;
	iss >> keyword;

	if (keyword == "Node")
	{
		parseNode(line, graph);
	}
	else if (keyword == "Rail")
	{
		parseRail(line, graph);
	}
	else
	{
		throw std::runtime_error("Unknown keyword '" + keyword + "'");
	}
}

void RailNetworkParser::parseNode(const std::string& line, Graph* graph)
{
	std::istringstream iss(line);
	std::string keyword;
	std::string nodeName;

	iss >> keyword >> nodeName;

	if (nodeName.empty())
	{
		throw std::runtime_error("Node name is empty");
	}

	if (graph->hasNode(nodeName))
	{
		throw std::runtime_error("Node '" + nodeName + "' already exists");
	}

	Node* node = new Node(nodeName);
	graph->addNode(node);
}

void RailNetworkParser::parseRail(const std::string& line, Graph* graph)
{
	std::istringstream iss(line);
	std::string keyword;
	std::string nodeAName;
	std::string nodeBName;
	double length;
	double speedLimit;

	iss >> keyword >> nodeAName >> nodeBName >> length >> speedLimit;

	if (iss.fail())
	{
		throw std::runtime_error(
			"Invalid Rail format. Expected: Rail <nodeA> <nodeB> <length> <speedLimit>"
		);
	}

	if (nodeAName.empty() || nodeBName.empty())
	{
		throw std::runtime_error("Rail node names cannot be empty");
	}

	if (!graph->hasNode(nodeAName))
	{
		throw std::runtime_error("Node '" + nodeAName + "' not found in network");
	}

	if (!graph->hasNode(nodeBName))
	{
		throw std::runtime_error("Node '" + nodeBName + "' not found in network");
	}

	if (length <= 0.0)
	{
		throw std::runtime_error(
			"Rail length must be positive, got " + std::to_string(length)
		);
	}

	if (speedLimit <= 0.0)
	{
		throw std::runtime_error(
			"Speed limit must be positive, got " + std::to_string(speedLimit)
		);
	}

	Node* nodeA = graph->getNode(nodeAName);
	Node* nodeB = graph->getNode(nodeBName);

	Rail* rail = new Rail(nodeA, nodeB, length, speedLimit);
	graph->addRail(rail);
}