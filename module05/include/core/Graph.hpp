#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <map>
#include <string>

class Node;
class Rail;

// Represents the complete railway network graph
class Graph
{
public:
	using NodeList = std::vector<Node*>;
	using RailList = std::vector<Rail*>;
	using AdjacencyMap = std::map<Node*, RailList>;

private:
	NodeList     _nodes;
	RailList     _rails;
	AdjacencyMap _adjacency;  // Node -> connected rails

	// Helper method
	bool nodeExistsInGraph(Node* node) const;

public:
	Graph() = default;
	Graph(const Graph&) = default;
	Graph& operator=(const Graph&) = default;
	~Graph() = default;

	// Node management
	void addNode(Node* node);
    Node* getNode(const std::string& name);
	const Node* getNode(const std::string& name) const;
	const NodeList& getNodes() const;
	bool hasNode(const std::string& name) const;
	size_t getNodeCount() const;

	// Rail management
	void addRail(Rail* rail);
	const RailList& getRails() const;
	size_t getRailCount() const;

	// Adjacency queries (for pathfinding)
	RailList getRailsFromNode(Node* node) const;
	std::vector<Node*> getNeighbors(Node* node) const;

	// Validation
	bool isValid() const;
    // void printAsciiMap() const;
	// Cleanup
	void clear();
};

#endif
