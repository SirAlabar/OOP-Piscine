#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "core/INetworkQuery.hpp"

class Node;
class Rail;

class Graph : public INetworkQuery
{
public:
	using NodeList = std::vector<Node*>;
	using RailList = std::vector<Rail*>;
	using AdjacencyMap = std::map<Node*, RailList>;

private:
	std::vector<std::unique_ptr<Node>> _nodes;
	std::vector<std::unique_ptr<Rail>> _rails;
	AdjacencyMap _adjacency;

	bool nodeExistsInGraph(Node* node) const;

public:
	Graph() = default;
	Graph(const Graph&) = delete;
	Graph& operator=(const Graph&) = delete;
	~Graph() = default;

	// Node management
	void addNode(Node* node);
	Node* getNode(const std::string& name);
	const Node* getNode(const std::string& name) const;
	NodeList getNodes() const;
	bool hasNode(const std::string& name) const;
	size_t getNodeCount() const;

	// Rail management
	void addRail(Rail* rail);
	RailList getRails() const;
	size_t getRailCount() const;

	// Adjacency queries
	RailList getRailsFromNode(Node* node) const;
	std::vector<Node*> getNeighbors(Node* node) const;

	// Validation
	bool isValid() const;
	void clear();
};

#endif