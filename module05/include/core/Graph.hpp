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
private:
	std::vector<Node*> _nodes;
	std::vector<Rail*> _rails;
	std::map<Node*, std::vector<Rail*>> _adjacency;  // Node -> connected rails

	// Helper method
	bool nodeExistsInGraph(Node* node) const;

public:
	Graph();
	Graph(const Graph& other);
	Graph& operator=(const Graph& other);
	~Graph();

	// Node management
	void  addNode(Node* node);
	Node* getNode(const std::string& name) const;
	const std::vector<Node*>& getNodes() const;
	bool  hasNode(const std::string& name) const;
	size_t getNodeCount() const;

	// Rail management
	void  addRail(Rail* rail);
	const std::vector<Rail*>& getRails() const;
	size_t getRailCount() const;

	// Adjacency queries (for pathfinding)
	std::vector<Rail*> getRailsFromNode(Node* node) const;
	std::vector<Node*> getNeighbors(Node* node) const;

	// Validation
	bool isValid() const;

	// Cleanup
	void clear();
};

#endif